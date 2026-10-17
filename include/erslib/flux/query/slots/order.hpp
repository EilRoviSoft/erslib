#pragma once

// ers
#include <erslib/flux/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot order {
        .name  = "flux.order",
        .arity = EArity::Multi,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding order {
        .slot      = &slots::order,
        .prefix    = "\nORDER BY ",
        .separator = ", ",
    };
}
