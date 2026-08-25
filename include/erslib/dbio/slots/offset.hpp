#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot offset {
        .name  = "dbio.offset",
        .arity = Arity::Single,
    };
}

namespace dbio::impl::bindings {
    inline constexpr SlotBinding offset {
        .slot   = &slots::offset,
        .prefix = "\nOFFSET ",
    };
}
