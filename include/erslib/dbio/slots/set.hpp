#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot set {
        .name  = "dbio.set",
        .arity = Arity::Multi,
    };
}

namespace dbio::impl::bindings {
    inline constexpr SlotBinding set {
        .slot      = &slots::set,
        .prefix    = "\nSET ",
        .separator = ", ",
    };
}
