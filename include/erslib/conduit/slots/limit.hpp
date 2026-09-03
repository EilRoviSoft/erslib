#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot limit {
        .name  = "conduit.limit",
        .arity = EArity::Single,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding limit {
        .slot   = &slots::limit,
        .prefix = "\nLIMIT ",
    };
}
