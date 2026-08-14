#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot values {
        .name      = "dbio.values",
        .prefix    = "\nVALUES (",
        .separator = "), (",
        .suffix    = ")",
        .arity     = Arity::Multi,
    };
}
