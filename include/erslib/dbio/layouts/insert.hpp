#pragma once

// std
#include <concepts>
#include <string>

// ers
#include <erslib/dbio/impl/query.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    namespace slots {
        inline constexpr Slot insert_into = {
            .name  = "dbio.insert_into",
            .arity = Arity::Single,
        };

        inline constexpr Slot insert_columns {
            .name  = "dbio.insert_columns",
            .arity = Arity::Multi,
        };
    }


    namespace layouts {
        ERSLIB_EXPORT Query insert_into(std::string table);
    }
}
