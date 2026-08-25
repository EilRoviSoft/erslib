#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot limit {
        .name  = "dbio.limit",
        .arity = Arity::Single,
    };
}

namespace dbio::impl::bindings {
    inline constexpr SlotBinding limit {
        .slot   = &slots::limit,
        .prefix = "\nLIMIT ",
    };
}
