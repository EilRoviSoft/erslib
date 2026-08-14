#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot column {
        .name  = "dbio.column",
        .arity = Arity::Multi
    };
}
