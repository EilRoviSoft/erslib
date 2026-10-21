#pragma once

// std
#include <string>
#include <string_view>
#include <tuple>

// export
#include <erslib/export.hpp>


// Stage is '<phase>-<index>'

namespace ers::modkit::impl {
    ERSLIB_EXPORT bool is_stage_naming_scheme(std::string_view sv);

    // Returns slice of 'sv'
    ERSLIB_EXPORT std::tuple<std::string, size_t> extract_stage_info(std::string_view sv);
}


// Exports ---------------------------------------------------------------------

namespace ers::modkit::util {
    using impl::is_stage_naming_scheme;
    using impl::extract_stage_info;
}
