#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot values {
        .name  = "dbio.values",
        .arity = Arity::Multi,
    };
}

namespace dbio::impl::bindings {
    inline constexpr SlotBinding values {
        .slot      = &slots::values,
        .prefix    = "\nVALUES (",
        .separator = "), (",
        .suffix    = ")"
    };
}
