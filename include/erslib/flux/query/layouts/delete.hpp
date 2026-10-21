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
        ERSLIB_EXPORT QueryBuilder delete_from(std::string table);
    }
}
