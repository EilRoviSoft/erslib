#pragma once

// std
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl::algo {
    ERSLIB_EXPORT Result<std::vector<std::string>> split_args(std::string_view line);
}
