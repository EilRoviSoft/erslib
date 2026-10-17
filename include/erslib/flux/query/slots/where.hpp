#pragma once

// ers
#include <erslib/flux/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot where {
        .name  = "flux.where",
        .arity = EArity::Multi,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding where {
        .slot      = &slots::where,
        .prefix    = "\nWHERE ",
        .separator = " AND ",
    };
}
