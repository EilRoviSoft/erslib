#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot distinct {
        .name  = "conduit.distinct",
        .arity = EArity::Single,
    };
}

namespace conduit::impl::bindings {
    inline constexpr SlotBinding distinct {
        .slot = &slots::distinct,
    };
}
