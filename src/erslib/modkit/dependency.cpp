#include "erslib/modkit/dependency.hpp"

// ers
#include <erslib/core/algorithm.hpp>
#include <erslib/modkit/mod.hpp>


// Dependency

ers::Result<ers::modkit::impl::dependency_t> ers::modkit::impl::dependency_t::parse(
    std::string_view source
) {
    dependency_t result;

    // Short openings first

    bool has_short_type = false;

    if (source.starts_with("?")) {
        result.type = EDependencyType::Optional;
        has_short_type = true;
    } else if (source.starts_with("!")) {
        result.type = EDependencyType::Incompatible;
        has_short_type = true;
    }

    if (has_short_type)
        source = source.substr(1);


    algo::RegularSplitter splitter(source, " ");
    auto it = splitter.begin();

    if (has_short_type) {
        bool has_long_type = false;

        if (*it == "opt") {
            result.type = EDependencyType::Optional;
            has_long_type = true;
        } else if (*it == "not") {
            result.type = EDependencyType::Incompatible;
            has_long_type = true;
        }

        if (has_long_type)
            it++;
    } else
        result.type = EDependencyType::Required;


    result.name = *it;

    if (++it == splitter.end())
        return result;


    bool has_version_bounds = false;

    if (*it == ">=" || *it == "min") {
        result.limit = EDependencyLimit::Lower;
        has_version_bounds = true;
    } else if (*it == "<=" || *it == "max") {
        result.limit = EDependencyLimit::Upper;
        has_version_bounds = true;
    }

    if (has_version_bounds) {
        it++;

        if (auto r = version_t::parse(*it); r)
            result.version = *r;
        else
            return r.error();
    }


    return result;
}
