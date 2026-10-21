#pragma once

// std
#include <concepts>
#include <string>

// ers
#include <erslib/flux/query/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    namespace slots {
        inline constexpr Slot insert_into = {
            .name  = "flux.insert_into",
            .arity = EArity::Single,
        };

        inline constexpr Slot insert_columns {
            .name  = "flux.insert_columns",
            .arity = EArity::Multi,
        };
    }


    namespace layouts {
        ERSLIB_EXPORT QueryBuilder insert_into(std::string table);
    }
}
