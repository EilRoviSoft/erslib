#pragma once

// std
#include <span>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl {
    ERSLIB_EXPORT Result<std::vector<std::string>> split_args(std::string_view line);

    ERSLIB_EXPORT std::string quote_arg(std::string_view arg);
    ERSLIB_EXPORT std::string join_args(const std::span<std::string>& args);
}
