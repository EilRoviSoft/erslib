#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    namespace layouts {
        ERSLIB_EXPORT QueryBuilder select_from(std::string table);
    }
}
