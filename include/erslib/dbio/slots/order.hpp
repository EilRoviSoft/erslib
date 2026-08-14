#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot order {
        .name  = "dbio.order",
        .arity = Arity::Multi,
    };
}
