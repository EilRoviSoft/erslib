#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    namespace slots {
        inline constexpr Slot delete_from = {
            .name  = "dbio.delete_from",
            .arity = Arity::Single,
        };
    }


    namespace layouts {
        QueryBuilder ERSLIB_EXPORT delete_from(std::string table);
    }
}
