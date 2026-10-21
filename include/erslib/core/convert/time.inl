#pragma once

// std
#include <sstream>

// ers
#include <erslib/core/exception.hpp>


namespace ers::impl {
    template<typename Clock, std::invocable OnError>
    std::invoke_result_t<OnError> parse_timestamp(std::string_view sv, std::string_view fmt, OnError&& on_error) {
        typename Clock::time_point result;
        std::istringstream iss{ std::string(sv) };

        if (!std::chrono::from_stream(iss, std::string(fmt).c_str(), result))
            return std::invoke(std::forward<OnError>(on_error));

        return result;
    }
}


namespace ers::impl {
    template<typename Clock>
    timestamp_t timestamp_from_string(std::string_view sv, std::string_view fmt) {
        return parse_timestamp<Clock>(sv, fmt, [&] -> timestamp_t {
            throw make_conversion_error("Can't cast '{}' with '{}'", sv, fmt);
        });
    }

    template<typename Clock>
    Result<timestamp_t> try_timestamp_from_string(std::string_view sv, std::string_view fmt) {
        return parse_timestamp<Clock>(sv, fmt, [&] -> Result<timestamp_t> {
            return make_error("Can't cast '{}' with '{}'", sv, fmt);
        });
    }

    template<typename Clock>
    maybe<timestamp_t> maybe_timestamp_from_string(std::string_view sv, std::string_view fmt) {
        return parse_timestamp<Clock>(sv, fmt, [] -> maybe<timestamp_t> {
            return none;
        });
    }
}
