#pragma once

// std
#include <cstdint>
#include <string_view>

// export
#include <erslib/export.hpp>


namespace dbio {
    enum class Section : uint8_t {
        Column,
        Where,
        OrderBy,
        Limit,
        Offset
    };
}

namespace dbio::internal {
    struct section_format_t {
        std::string_view prefix;
        std::string_view separator;
    };

    ERSLIB_EXPORT bool is_singular(Section section);
    ERSLIB_EXPORT bool is_identifier(std::string_view name);
}
