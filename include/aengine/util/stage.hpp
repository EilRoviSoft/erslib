#pragma once

// std
#include <string>


// Stage is '<phase>-<index>'

namespace aengine::util {
    bool is_stage_naming_scheme(std::string_view sv);

    // Returns slice of 'sv'
    std::tuple<std::string, size_t> extract_stage_info(std::string_view sv);
}
