#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot where {
        .name  = "dbio.where",
        .arity = Arity::Multi,
    };
}

namespace dbio::impl::bindings {
    inline constexpr SlotBinding where {
        .slot      = &slots::where,
        .prefix    = "\nWHERE ",
        .separator = " AND "
    };
}
