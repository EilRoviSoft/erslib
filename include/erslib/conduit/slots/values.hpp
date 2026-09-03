#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot values {
        .name  = "conduit.values",
        .arity = EArity::Multi,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding values {
        .slot      = &slots::values,
        .prefix    = "\nVALUES (",
        .separator = "), (",
        .suffix    = ")",
    };
}
