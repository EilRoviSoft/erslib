#pragma once

// ers
#include <erslib/flux/impl/slot.hpp>


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
