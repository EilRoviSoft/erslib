#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot from {
        .name  = "conduit.from",
        .arity = EArity::Multi,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding from {
        .slot      = &slots::from,
        .prefix    = "\nFROM ",
        .separator = ", ",
    };
}
