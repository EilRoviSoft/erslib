#pragma once

// ers
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/time.hpp>


// Details ---------------------------------------------------------------------

namespace ers::impl {
    static constexpr std::string_view default_timestamp_fmt = "%Y-%m-%dT%H:%M:%S";
}


// Implementation --------------------------------------------------------------

namespace ers::impl {
    template<typename Clock>
    timestamp_t timestamp_from_string(std::string_view sv, std::string_view fmt);

    template<typename Clock>
    Result<timestamp_t> try_timestamp_from_string(std::string_view sv, std::string_view fmt);

    template<typename Clock>
    maybe<timestamp_t> maybe_timestamp_from_string(std::string_view sv, std::string_view fmt);


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
    using impl::try_timestamp_from_string;
    using impl::maybe_timestamp_from_string;

    using impl::timestamp_from_local;
    using impl::timestamp_to_local;
}


// Implementation --------------------------------------------------------------

#include "time.inl"
