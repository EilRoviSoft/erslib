#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot order {
        .name  = "conduit.order",
        .arity = EArity::Multi,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding order {
        .slot      = &slots::order,
        .prefix    = "\nORDER BY ",
        .separator = ", ",
    };
}
