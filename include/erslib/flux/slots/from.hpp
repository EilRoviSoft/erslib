#pragma once

// ers
#include <erslib/flux/impl/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot from {
        .name  = "flux.from",
        .arity = EArity::Multi,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding from {
        .slot      = &slots::from,
        .prefix    = "\nFROM ",
        .separator = ", ",
    };
}
