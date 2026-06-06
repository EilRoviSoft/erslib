#include "aengine/core/dependency.hpp"

// ers
#include <erslib/formatter/include.hpp>
#include <erslib/splitting/regular.hpp>

// aengine
#include <aengine/core/mod.hpp>


// Dependency

ers::Result<aengine::dependency_t> ers::convert::from_string_backend<aengine::dependency_t>::runtime_value(
    std::string_view source
) const {
    using namespace aengine;


    dependency_t result;

    // Short openings first

    bool has_short_type = false;

    if (source.starts_with("?")) {
        result.type = DependencyType::Optional;
        has_short_type = true;
    } else if (source.starts_with("!")) {
        result.type = DependencyType::Incompatible;
        has_short_type = true;
    }

    if (has_short_type)
        source = source.substr(1);


    RegularSplitter splitter(source, " ");
    auto it = splitter.begin();

    if (has_short_type) {
        bool has_long_type = false;

        if (*it == "opt") {
            result.type = DependencyType::Optional;
            has_long_type = true;
        } else if (*it == "not") {
            result.type = DependencyType::Incompatible;
            has_long_type = true;
        }

        if (has_long_type)
            it++;
    } else
        result.type = DependencyType::Required;


    result.name = *it;

    if (++it == splitter.end())
        return result;


    bool has_version_bounds = false;

    if (*it == ">=" || *it == "min") {
        result.limit = DependencyLimit::Lower;
        has_version_bounds = true;
    } else if (*it == "<=" || *it == "max") {
        result.limit = DependencyLimit::Upper;
        has_version_bounds = true;
    }

    if (has_version_bounds) {
        it++;

        if (auto r = convert::from_str<version_t>(*it); r)
            result.version = *r;
        else
            return r.error();
    }


    return result;
}
