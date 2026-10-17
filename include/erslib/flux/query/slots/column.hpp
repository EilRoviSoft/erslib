#pragma once

// ers
#include <erslib/flux/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot column {
        .name  = "flux.column",
        .arity = EArity::Multi,
    };
}
