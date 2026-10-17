#pragma once

// std
#include <string>

// ers
#include <erslib/flux/query/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    namespace slots {
        inline constexpr Slot delete_from = {
            .name  = "flux.delete_from",
            .arity = EArity::Single,
        };
    }


    namespace layouts {
        QueryBuilder ERSLIB_EXPORT delete_from(std::string table);
    }
}
