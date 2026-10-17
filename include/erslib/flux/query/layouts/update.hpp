#pragma once

// std
#include <string>

// ers
#include <erslib/flux/query/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    namespace slots {
        inline constexpr Slot update {
            .name  = "flux.update",
            .arity = EArity::Single,
        };
    }


    namespace layouts {
        QueryBuilder ERSLIB_EXPORT update(std::string table);
    }
}
