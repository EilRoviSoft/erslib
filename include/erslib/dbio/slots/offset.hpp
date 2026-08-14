#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot offset {
        .name   = "dbio.offset",
        .prefix = "\nOFFSET ",
        .arity  = Arity::Single,
    };
}
