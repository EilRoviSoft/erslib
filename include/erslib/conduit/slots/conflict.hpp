#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot conflict {
        .name  = "conduit.conflict",
        .arity = EArity::Multi,
    };

    inline constexpr Slot conflict_constraint {
        .name  = "conduit.conflict_constraint",
        .arity = EArity::Single,
    };

    inline constexpr Slot conflict_action {
        .name  = "conduit.conflict_action",
        .arity = EArity::Single,
    };
}

namespace conduit::impl::bindings {
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
