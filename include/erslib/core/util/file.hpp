#pragma once

// std
#include <filesystem>
#include <string>

// export
#include <erslib/export.hpp>


namespace ers::impl::util {
    ERSLIB_EXPORT std::string read_file(const std::filesystem::path& path);
}
