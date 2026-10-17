#pragma once

// ers
#include <erslib/flux/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot conflict {
        .name  = "flux.conflict",
        .arity = EArity::Multi,
    };

    inline constexpr Slot conflict_constraint {
        .name  = "flux.conflict_constraint",
        .arity = EArity::Single,
    };

    inline constexpr Slot conflict_action {
        .name  = "flux.conflict_action",
        .arity = EArity::Single,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding conflict {
        .slot      = &slots::conflict,
        .prefix    = "\nON CONFLICT (",
        .separator = ", ",
        .suffix    = ")",
    };

    inline constexpr SlotBinding conflict_constraint {
        .slot   = &slots::conflict_constraint,
        .prefix = "\nON CONFLICT ON CONSTRAINT ",
    };

    inline constexpr SlotBinding conflict_action {
        .slot   = &slots::conflict_action,
        .prefix = "\n",
    };
}
