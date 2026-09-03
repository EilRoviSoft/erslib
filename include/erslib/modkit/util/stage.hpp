#pragma once

// std
#include <string>


// Stage is '<phase>-<index>'

namespace ers::modkit::impl {
    bool is_stage_naming_scheme(std::string_view sv);

    // Returns slice of 'sv'
    std::tuple<std::string, size_t> extract_stage_info(std::string_view sv);
}


// Exports

namespace ers::modkit::util {
    using impl::is_stage_naming_scheme;
    using impl::extract_stage_info;
}
