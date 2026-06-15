#include "aengine/core/mod.hpp"

// std
#include <ranges>

// ers
#include <contrib/json.hpp>
#include <erslib/io/json_schema.hpp>
#include <erslib/util/file.hpp>
#include <erslib/util/string.hpp>

// aescript
#include <aescript/error.hpp>
#include <aescript/exception.hpp>


using std::string_literals::operator ""s;
using namespace aescript;


// Internal

namespace {
    ers::Result<aengine::DependencyContainer> parse_dependencies(const utl::Json::array_type& array) {
        aengine::DependencyContainer result;

        for (const auto& obj : array) {
            if (!obj.is_string()) {
                return ers::make_error(
                    ers::Severity::Error,
                    "Expected '{}' got '{}'.",
                    ers::meta::type_name_v<utl::Json::string_type>, ers::convert::to_str(obj.type())
                );
            }

            auto r = ers::convert::from_str<aengine::dependency_t>(obj.as_string());
            if (!r)
                return r.error();

            result.emplace(*r);
        }

        return result;
    }


    std::string path_to_package_name(const fs::path& path) {
        auto temp = path.generic_string();
        temp = temp.substr(0, temp.size() - path.extension().string().size());
        return ers::util::replace(temp, "/", ".");
    }


    auto extract_info(const fs::path& m_dir) {
        aengine::internal::ModIdentity identity;
        aengine::internal::ModMetadata metadata;


        fs::path info_path = m_dir / "info.json";

        if (!fs::exists(info_path)) {
            throw ers::make_path_error("Mod directory '{}' without 'info.json' is specified",
                info_path.string());
        }

        if (!fs::is_regular_file(info_path)) {
            throw ers::make_path_error("Path '{}' doesn't lead to file",
                info_path.string());
        }


        utl::Json json;

        try {
            json = utl::from_file(info_path.string());
        } catch (const fs::filesystem_error& e) {
            throw ers::make_path_error("Error during parsing json with path '{}' occured. Info: {}",
                info_path.string(), e.what());
        }


        ers::JsonSchema schema(json);

        schema.require_and_write("name", identity.name);
        schema.require_and_write("title", identity.title);
        schema.require_and_convert("version", identity.version, &ers::convert::from_str<ers::version_t>);
        schema.require_and_write("author", metadata.author);
        schema.write_if_exist("contact", metadata.contact);
        schema.require_and_write("description", metadata.description);
        schema.require_and_convert("dependencies", metadata.dependencies, &parse_dependencies);

        if (auto s = schema.finalize(); !s)
            throw ers::runtime_error(s.error().to_string());


        if (auto folder_name = m_dir.stem().string(); folder_name != identity.name) {
            throw ers::make_path_error("Mod name ('{}') should be the same as folder name ('{}').",
                identity.name, folder_name);
        }


        return std::make_tuple(std::move(identity), std::move(metadata));
    }
}


// Mod

aengine::Mod::Mod(fs::path dir) :
    m_dir(std::move(dir)) {
}


void aengine::Mod::init_info() {
    auto [identity, metadata] = extract_info(m_dir);

    m_identity = std::move(identity);
    m_metadata = std::make_unique<internal::ModMetadata>(std::move(metadata));
}

void aengine::Mod::init_content(const std::function<bool(std::string_view)>& stage_filter) const {
    content_type content;

    for (const auto& it : fs::recursive_directory_iterator(m_dir)) {
        auto stem = it.path().stem().generic_string();
        auto extension = it.path().extension().generic_string();

        if (extension != ".lua")
            continue;

        // If file isn't stage, it should be written as a package
        // and be available later as include.

        if (stage_filter(stem))
            content.stages.emplace(stem, ers::util::read_file(it));
        else
            content.packages.emplace(path_to_package_name(fs::relative(it, m_dir)), ers::util::read_file(it));
    }

    m_content = std::make_unique<content_type>(std::move(content));
}

void aengine::Mod::init_runtime(sol::state_view& lua) const {
    runtime_type runtime;

    {
        sol::environment env(lua, sol::create, lua.globals());

        env["__mod_name"] = m_identity.name;

        env["require"] = _make_require_fn();

        static constexpr auto nil_fields = { "dofile", "loadfile", "load", "loadstring", "io", "os" };
        for (const auto& it : nil_fields)
            env[it] = sol::nil;

        runtime.env = std::move(env);
    }


    m_runtime = std::make_unique<runtime_type>(std::move(runtime));
}

void aengine::Mod::load_runtime(std::string_view stage_name) const {
    auto it = content().stages.find(stage_name);
    if (it == content().stages.end())
        throw make_lua_stage_error("Stage {} is not found", stage_name);


    sol::state_view lua = m_runtime->env.lua_state();
    sol::load_result chunk = lua.load(it->second, std::format("{}:{}", m_identity.name, stage_name));

    if (!chunk.valid()) {
        sol::error e = chunk;
        throw make_lua_stage_error("Failed to compile stage '{}': {}",
            stage_name, e.what());
    }

    sol::protected_function fn = chunk;

    if (!m_runtime->env.set_on(fn)) {
        throw make_lua_stage_error("Failed to set environment for stage '{}'",
            stage_name);
    }


    sol::protected_function_result result = fn();

    if (!result.valid()) {
        if (m_runtime->pending_exception) {
            auto ex = m_runtime->pending_exception;
            m_runtime->pending_exception = nullptr;
            std::rethrow_exception(ex);
        }

        sol::error e = result;
        throw make_lua_stage_error("Failed to execute stage '{}': {}",
            stage_name, e.what());
    }
}


std::function<sol::object(sol::this_state, std::string_view)> aengine::Mod::_make_require_fn() const {
    return [this](sol::this_state ts, std::string_view package_name) -> sol::object {
        sol::state_view lua = ts.lua_state();

        
        auto& content = *m_content;
        auto& runtime = *m_runtime;

        if (auto cache_it = runtime.modules_cache.find(package_name); cache_it != runtime.modules_cache.end())
            return cache_it->second;

        auto package_it = content.packages.find(package_name);
        if (package_it == content.packages.end()) {
            runtime.pending_exception = tunnel_error<lua_package_error>(lua, "Package {} is not found",
                package_name);
            luaL_error(lua, "C++ exception is tunneled");
            return sol::nil;
        }

        runtime.modules_cache.emplace(package_name, sol::make_object(lua, true));


        auto lr = lua.load(package_it->second, std::format("{}:{}", m_identity.name, package_name));

        if (!lr.valid()) {
            sol::error e = lr;
            runtime.pending_exception = tunnel_error<lua_package_error>(lua, "Failed to compile package '{}': {}",
                package_name, e.what());
            luaL_error(lua, "C++ exception is tunneled");
            return sol::nil;
        }


        sol::protected_function pf = lr;

        if (!runtime.env.set_on(pf)) {
            runtime.pending_exception = tunnel_error<lua_package_error>(lua, "Failed to set environment for package '{}'",
                package_name);
            luaL_error(lua, "C++ exception is tunneled");
            return sol::nil;
        }


        auto result = pf();

        if (!result.valid()) {
            sol::error e = result;
            runtime.pending_exception = tunnel_error<lua_package_error>(lua, "Failed to execute package '{}': {}",
                package_name, e.what());
            luaL_error(lua, "C++ exception is tunneled");
            return sol::nil;
        }


        sol::object module = result.return_count() > 0
            ? result.get<sol::object>()
            : sol::make_object(lua, true);

        runtime.modules_cache[package_name] = module;

        return module;
    };
}
