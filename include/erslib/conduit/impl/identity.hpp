#pragma once

// std
#include <string_view>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    ERSLIB_EXPORT bool is_identifier(std::string_view name);
}
