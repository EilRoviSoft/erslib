#pragma once

// std
#include <filesystem>

namespace fs = std::filesystem;

namespace ers {
    constexpr char path_separator = fs::path::preferred_separator;
}
