#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot group {
        .name  = "conduit.group",
        .arity = EArity::Multi,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding group {
        .slot      = &slots::group,
        .prefix    = "\nGROUP BY ",
        .separator = ", ",
    };
}
