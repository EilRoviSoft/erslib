#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot where {
        .name  = "conduit.where",
        .arity = EArity::Multi,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding where {
        .slot      = &slots::where,
        .prefix    = "\nWHERE ",
        .separator = " AND ",
    };
}
