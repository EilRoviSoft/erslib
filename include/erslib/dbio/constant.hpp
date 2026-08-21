#pragma once

// std
#include <limits>

// ers
#include <erslib/core/type/general.hpp>


namespace dbio::impl {
    // Sentinel for not-yet-persisted identity columns (GENERATED ALWAYS AS IDENTITY).
    inline constexpr u32 undefined_id = std::numeric_limits<u32>::max();
}


// Exports

namespace dbio {
    using impl::undefined_id;
}
