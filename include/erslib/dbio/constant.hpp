#pragma once

// std
#include <cstdint>
#include <limits>


namespace dbio {
    // Sentinel for not-yet-persisted identity columns (GENERATED ALWAYS AS IDENTITY).
    // Generated entities default their identity field to this until a save/load assigns the real value.
    inline constexpr std::uint32_t undefined_id = std::numeric_limits<std::uint32_t>::max();
}
