#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot join {
        .name  = "conduit.join",
        .arity = EArity::Multi,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding join {
        .slot = &slots::join
    };
}
