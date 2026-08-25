#pragma once

// std
#include <sstream>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/time.hpp>


// Implementation

namespace ers::impl {
    template<typename Clock>
    Result<timestamp_t> timestamp_from_string(std::string_view sv, std::string_view fmt) {
        typename Clock::time_point result;
        std::stringstream iss(static_cast<std::string>(sv));

        if (!std::chrono::from_stream(iss, (static_cast<std::string>(fmt) + '\0').c_str(), result))
            return make_error("Can't cast '{}' with '{}'", sv, fmt);

        return result;
    }
}


// Exports

namespace ers {
    using impl::timestamp_from_string;
}
