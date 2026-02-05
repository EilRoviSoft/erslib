#pragma once

// std
#include <sstream>

// ers
#include <erslib/error/result.hpp>
#include <erslib/type/time.hpp>

namespace ers {
    template<typename Clock>
    Result<timestamp_t> from_chars(std::string_view sv, std::string_view fmt) {
        typename Clock::time_point result;
        std::stringstream iss(static_cast<std::string>(sv));

        if (!std::chrono::from_stream(iss, (static_cast<std::string>(fmt) + '\0').c_str(), result)) {
            return Unexpected(
                Severity::Error,
                "bad_time_cast",
                std::format("Can't cast \"{}\" with \"{}\"", sv, fmt)
            );
        }

        return result;
    }
}
