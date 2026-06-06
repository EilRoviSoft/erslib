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
                    "json_type_error",
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
}


// ModContent::phase_order

bool aengine::internal::ModContent::phase_order::operator()(std::string_view lhs, std::string_view rhs) const {
    Phase p_lhs = *ers::convert::from_str_constexpr<Phase>(lhs);
    Phase p_rhs = *ers::convert::from_str_constexpr<Phase>(rhs);

    if (p_lhs != p_rhs)
        return p_lhs < p_rhs;

    auto extract = [](std::string_view sv, Phase phase) {
        size_t off = ers::convert::from_string_backend<Phase>::conversion_table[static_cast<size_t>(phase) - 1].first.size() + 1;
        auto r = ers::convert::from_str<size_t>(sv.substr(off));
        return r ? *r : throw ers::conversion_error(r.error().to_string(true));
    };

    return extract(lhs, p_lhs) < extract(rhs, p_rhs);
}


// ModContent

namespace {
    std::string path_to_package_name(const fs::path& path) {
        auto temp = path.generic_string();
        temp = temp.substr(0, temp.size() - path.extension().string().size());
        return ers::util::replace(temp, "/", ".");
    }
}


void aengine::internal::ModContent::load(const fs::path& dir) {
    for (const auto& it : fs::recursive_directory_iterator(dir)) {
        auto stem = it.path().stem().generic_string();
        auto extension = it.path().extension().generic_string();

        if (extension != ".lua")
            continue;

        // If file isn't stage, it should be written as a package
        // and be available later as include.

        if (*ers::convert::from_str<Phase>(stem) != Phase::Unknown)
            m_stages.emplace(stem, ers::util::read_file(it));
        else
            m_packages.emplace(path_to_package_name(fs::relative(it, dir)), ers::util::read_file(it));
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
    m_content = std::make_unique<internal::ModContent>();
}


void aengine::Mod::drop_metadata() const {
    std::ignore = m_metadata.release();
}
void aengine::Mod::drop_content() const {
    std::ignore = m_content.release();
}
