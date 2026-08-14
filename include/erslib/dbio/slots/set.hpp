#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot set {
        .name      = "dbio.set",
        .prefix    = "\nSET ",
        .separator = ", ",
        .arity     = Arity::Multi,
    };
}
