#pragma once

// std
#include <limits>

// ers
#include <erslib/core/type/general.hpp>


namespace conduit::impl {
    // Sentinel for not-yet-persisted identity columns (GENERATED ALWAYS AS IDENTITY).
    inline constexpr u32 undefined_id = std::numeric_limits<u32>::max();
}
