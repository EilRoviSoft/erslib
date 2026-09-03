#pragma once

// std
#include <concepts>
#include <string>

// ers
#include <erslib/conduit/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    namespace slots {
        inline constexpr Slot insert_into = {
            .name  = "conduit.insert_into",
            .arity = EArity::Single,
        };

        inline constexpr Slot insert_columns {
            .name  = "conduit.insert_columns",
            .arity = EArity::Multi,
        };
    }


    namespace layouts {
        QueryBuilder ERSLIB_EXPORT insert_into(std::string table);
    }
}
