#pragma once

// std
#include <filesystem>
#include <string>

// export
#include <erslib/export.hpp>


namespace ers::impl::util {
    std::string ERSLIB_EXPORT read_file(const std::filesystem::path& path);
}
