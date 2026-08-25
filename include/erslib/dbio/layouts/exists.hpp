#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    namespace layouts {
        // SELECT EXISTS (SELECT 1 FROM <table> [WHERE ...]) AS exists
        QueryBuilder ERSLIB_EXPORT exists_in(std::string table);
    }
}
