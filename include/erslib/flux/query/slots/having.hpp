#pragma once

// ers
#include <erslib/flux/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot having {
        .name  = "flux.having",
        .arity = EArity::Multi,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding having {
        .slot      = &slots::having,
        .prefix    = "\nHAVING ",
        .separator = " AND ",
    };
}
