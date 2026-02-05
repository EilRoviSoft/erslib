#pragma once

// std
#include <chrono>

namespace ers {
    using clock_t = std::chrono::system_clock;
    using timestamp_t = std::chrono::system_clock::time_point;
    using duration_t = std::chrono::system_clock::duration;
}
