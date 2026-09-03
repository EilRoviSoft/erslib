#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot set {
        .name  = "conduit.set",
        .arity = EArity::Multi,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding set {
        .slot      = &slots::set,
        .prefix    = "\nSET ",
        .separator = ", ",
    };
}
