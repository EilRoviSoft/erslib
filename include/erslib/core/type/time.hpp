#pragma once

// std
#include <chrono>


namespace ers::impl {
    using clock_t = std::chrono::system_clock;
    using timestamp_t = std::chrono::system_clock::time_point;
    using duration_t = std::chrono::system_clock::duration;

    using ms_t = std::chrono::milliseconds;
}


// Exports

namespace ers {
    using impl::clock_t;
    using impl::timestamp_t;
    using impl::duration_t;

    using impl::ms_t;
}
