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
    Result<std::vector<std::string>> ERSLIB_EXPORT split_args(std::string_view line);
}
