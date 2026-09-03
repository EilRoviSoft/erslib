#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot offset {
        .name  = "conduit.offset",
        .arity = EArity::Single,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding offset {
        .slot   = &slots::offset,
        .prefix = "\nOFFSET ",
    };
}
