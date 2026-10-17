#pragma once

// ers
#include <erslib/flux/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot limit {
        .name  = "flux.limit",
        .arity = EArity::Single,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding limit {
        .slot   = &slots::limit,
        .prefix = "\nLIMIT ",
    };
}
