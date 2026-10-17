#pragma once

// std
#include <sstream>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/time.hpp>


// Details ---------------------------------------------------------------------

namespace ers::impl {
    static constexpr std::string_view default_timestamp_fmt = "%Y-%m-%dT%H:%M:%S";
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    template<typename Clock>
    Result<timestamp_t> timestamp_from_string(std::string_view sv, std::string_view fmt) {
        typename Clock::time_point result;
        std::stringstream iss(static_cast<std::string>(sv));

        if (!std::chrono::from_stream(iss, (static_cast<std::string>(fmt) + '\0').c_str(), result))
            return make_error("Can't cast '{}' with '{}'", sv, fmt);

        return result;
    }


    Result<timestamp_t> timestamp_from_local(
        std::string_view text,
        const std::chrono::time_zone& zone,
        std::string_view fmt = default_timestamp_fmt
    );

    std::string timestamp_to_local(
        timestamp_t what,
        const std::chrono::time_zone& zone,
        std::string_view fmt = default_timestamp_fmt
    );
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::timestamp_from_string;

    using impl::timestamp_from_local;
    using impl::timestamp_to_local;
}
