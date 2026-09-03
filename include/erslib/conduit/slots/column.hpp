#pragma once

// ers
#include <erslib/conduit/impl/slot.hpp>


namespace conduit::impl::slots {
    inline constexpr Slot column {
        .name  = "conduit.column",
        .arity = EArity::Multi,
    };
}
