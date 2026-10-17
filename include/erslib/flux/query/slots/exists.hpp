#pragma once

// ers
#include <erslib/flux/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot exists_target {
        .name  = "flux.exists_target",
        .arity = EArity::Single,
    };

    inline constexpr Slot exists_alias {
        .name  = "flux.exists_alias",
        .arity = EArity::Single,
    };
}
