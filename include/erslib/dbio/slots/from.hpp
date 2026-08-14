#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot from {
        .name      = "dbio.from",
        .prefix    = "\nFROM ",
        .separator = ", ",
        .arity     = Arity::Multi,
    };
}
