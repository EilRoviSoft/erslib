#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot exists_target {
        .name  = "dbio.exists_target",
        .arity = Arity::Single,
    };

    inline constexpr Slot exists_alias {
        .name  = "dbio.exists_alias",
        .arity = Arity::Single,
    };
}
