#pragma once

// ers
#include <erslib/flux/impl/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot returning {
        .name  = "flux.returning",
        .arity = EArity::Multi,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding returning {
        .slot      = &slots::returning,
        .prefix    = "\nRETURNING ",
        .separator = ", ",
    };
}
