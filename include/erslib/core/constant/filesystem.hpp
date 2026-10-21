#pragma once

// std
#include <filesystem>


namespace ers::impl {
    constexpr char path_separator = std::filesystem::path::preferred_separator;
}
