#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot from {
        .name  = "dbio.from",
        .arity = Arity::Multi,
    };
}

namespace dbio::impl::bindings {
    inline constexpr SlotBinding from {
        .slot      = &slots::from,
        .prefix    = "\nFROM ",
        .separator = ", ",
    };
}
