#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot limit {
        .name   = "dbio.limit",
        .prefix = "\nLIMIT ",
        .arity  = Arity::Single,
    };
}
