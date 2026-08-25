#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot returning {
        .name  = "dbio.returning",
        .arity = Arity::Multi,
    };
}

namespace dbio::impl::bindings {
    inline constexpr SlotBinding returning {
        .slot      = &slots::returning,
        .prefix    = "\nRETURNING ",
        .separator = ", ",
    };
}
