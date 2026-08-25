#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    namespace layouts {
        QueryBuilder ERSLIB_EXPORT select_from(std::string table, std::string alias = "");

        // SELECT with no FROM
        QueryBuilder ERSLIB_EXPORT select();
    }
}
