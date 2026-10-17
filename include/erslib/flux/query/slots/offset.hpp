#pragma once

// ers
#include <erslib/flux/query/slot.hpp>


namespace flux::impl::slots {
    inline constexpr Slot offset {
        .name  = "flux.offset",
        .arity = EArity::Single,
    };
}

namespace flux::impl::bindings {
    inline constexpr SlotBinding offset {
        .slot   = &slots::offset,
        .prefix = "\nOFFSET ",
    };
}
