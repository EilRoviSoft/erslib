#pragma once

// ers
#include <erslib/dbio/impl/slot.hpp>


namespace dbio::impl::slots {
    inline constexpr Slot distinct {
        .name  = "dbio.distinct",
        .arity = Arity::Single,
    };
}

namespace dbio::impl::bindings {
    inline constexpr SlotBinding distinct {
        .slot = &slots::distinct,
    };
}
