#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot returning {
        .name  = "conduit.returning",
        .arity = EArity::Multi,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding returning {
        .slot      = &slots::returning,
        .prefix    = "\nRETURNING ",
        .separator = ", ",
    };
}
