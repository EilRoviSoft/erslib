#include "aengine/core/mod.hpp"

// std
#include <ranges>

// ers
#include <contrib/json.hpp>
#include <erslib/io/json_schema.hpp>
#include <erslib/util/file.hpp>
#include <erslib/util/string.hpp>

// aengine
#include <aengine/enum/phase.hpp>


using std::string_literals::operator ""s;


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
}


// Mod

aengine::Mod::Mod(fs::path dir) :
    m_dir(std::move(dir)) {
}


void aengine::Mod::load_info() {
    fs::path path = m_dir / "info.json";

    if (!fs::exists(path)) {
        throw ers::path_error("Mod directory '{}' without 'info.json' is specified", path.string());
    }

    if (!fs::is_regular_file(path)) {
        throw ers::path_error("Path '{}' doesn't lead to file", path.string());
    }


    utl::Json json;

    try {
        json = utl::from_file(path.string());
    } catch (const fs::filesystem_error& e) {
        throw ers::path_error("Error during parsing json with path '{}' occured. Info: {}", path.string(), e.what());
    }


    ers::JsonSchema schema(json);

    schema.require_and_write("name", m_identity.name);
    schema.require_and_write("title", m_identity.title);
    schema.require_and_convert("version", m_identity.version, &ers::convert::from_str<ers::version_t>);
    schema.require_and_write("author", m_metadata->author);
    schema.write_if_exist("contact", m_metadata->contact);
    schema.require_and_write("description", m_metadata->description);
    schema.require_and_convert("dependencies", m_metadata->dependencies, &parse_dependencies);


    if (auto s = schema.finalize(); !s)
        throw ers::runtime_error(s.error().to_string());


    if (auto folder_name = m_dir.stem().string(); folder_name != m_identity.name) {
        throw ers::path_error("Mod name ('{}') should be the same as folder name ('{}').",
            m_identity.name, folder_name);
    }
}

void aengine::Mod::load_content() const {
    content_type content;

    for (const auto& it : fs::recursive_directory_iterator(m_dir)) {
        auto stem = it.path().stem().generic_string();
        auto extension = it.path().extension().generic_string();

        if (extension != ".lua")
            continue;

        // If file isn't stage, it should be written as a package
        // and be available later as include.

        if (*ers::convert::from_str<Phase>(stem) != Phase::Unknown)
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

        env["__mod_name"] = name();

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
        throw internal::lua_stage_error("Stage {} is not found", stage_name);


    sol::state_view lua(m_runtime->env.lua_state());
    sol::load_result chunk = lua.load(it->second, std::format("{}:{}", name(), stage_name));

    if (!chunk.valid() || !m_runtime->env.set_on(chunk)) {
        sol::error e = chunk;
        throw internal::lua_stage_error("Failed to compile stage '{}': {}",
            stage_name, e.what());
    }


    chunk();
}


std::function<sol::object(std::string_view)> aengine::Mod::_make_require_fn() const {
    return [this](std::string_view package_name) -> sol::object {
        auto& content = *m_content;
        auto& runtime = *m_runtime;

        if (auto cache_it = runtime.modules_cache.find(package_name); cache_it != runtime.modules_cache.end())
            return cache_it->second;

        auto package_it = content.packages.find(package_name);
        if (package_it == content.packages.end()) {
            throw internal::lua_package_error("Package {} is not found", package_name);
        }


        sol::state_view lua = runtime.env.lua_state();

        runtime.modules_cache.emplace(package_name, sol::make_object(lua, true));


        auto lr = lua.load(package_it->second);

        if (!lr.valid()) {
            sol::error e = lr;
            throw internal::lua_package_error("Failed to compile package '{}': {}",
                package_name, e.what());
        }


        auto result = lr(runtime.env);
        sol::object module = result.return_count() > 0
            ? result.get<sol::object>()
            : sol::make_object(lua, true);

        runtime.modules_cache[package_name] = module;

        return module;
    };
}
