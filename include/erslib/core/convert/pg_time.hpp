#pragma once

// std
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/time.hpp>

// export
#include <erslib/export.hpp>


// Text conversion for PostgreSQL TIMESTAMPTZ (DateStyle = ISO, the default).
//
// Output:
// 'YYYY-MM-DD HH:MM:SS.ffffff+00' (always UTC, always 6 fraction digits,
// ' BC' suffix for years <= 0, 'infinity' / '-infinity' for max() / min()).
// 
// Input:
// 'YYYY-MM-DD HH:MM:SS[.f{1,6}]{+|-}HH[:MM[:SS]][ BC]', 'infinity', '-infinity'.
// 'T' as the separator and a 'Z' offset are accepted as well.


namespace ers::impl {
    ERSLIB_EXPORT timestamp_t floor_to_pg_precision(timestamp_t what);
    ERSLIB_EXPORT std::string timestamp_to_iso8601(timestamp_t what);
    ERSLIB_EXPORT Result<timestamp_t> timestamp_from_iso8601(std::string_view text);

    ERSLIB_EXPORT timestamp_t pg_now();
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::floor_to_pg_precision;
    using impl::timestamp_from_iso8601;
    using impl::timestamp_to_iso8601;

    using impl::pg_now;
}
