#pragma once

// std
#include <string>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl {
    // "sub/new_order.txt" -> "txt.sub.new_order", the path is relative to a first-level folder
    ERSLIB_EXPORT Result<std::string> make_localization_key(const fs::path& relative);
}
