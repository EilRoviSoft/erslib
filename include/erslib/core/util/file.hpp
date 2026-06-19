#pragma once

// std
#include <filesystem>
#include <string>

// export
#include <erslib/core/export.hpp>


namespace fs = std::filesystem;

namespace ers::util {
    std::string ERSLIB_EXPORT read_file(const fs::path& path);
}
