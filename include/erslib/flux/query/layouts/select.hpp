#pragma once

// std
#include <string>

// ers
#include <erslib/flux/query/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    namespace layouts {
        ERSLIB_EXPORT QueryBuilder select_from(std::string table, std::string alias = "");

        // SELECT with no FROM
        ERSLIB_EXPORT QueryBuilder select();
    }
}
