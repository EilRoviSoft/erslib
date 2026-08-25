#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot conflict {
        .name  = "dbio.conflict",
        .arity = Arity::Multi,
    };

    inline constexpr Slot conflict_constraint {
        .name  = "dbio.conflict_constraint",
        .arity = Arity::Single,
    };

    inline constexpr Slot conflict_action {
        .name  = "dbio.conflict_action",
        .arity = Arity::Single,
    };
}

namespace dbio::impl::bindings {
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
