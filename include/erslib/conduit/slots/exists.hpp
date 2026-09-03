#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot exists_target {
        .name  = "conduit.exists_target",
        .arity = EArity::Single,
    };

    inline constexpr Slot exists_alias {
        .name  = "conduit.exists_alias",
        .arity = EArity::Single,
    };
}
