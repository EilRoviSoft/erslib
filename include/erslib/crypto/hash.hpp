#pragma once

// std
#include <cstddef>
#include <span>
#include <string>
#include <string_view>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace ers::crypto::impl {
    ERSLIB_EXPORT std::string sha256_hex(std::span<const std::byte> bytes);
    ERSLIB_EXPORT std::string sha256_hex(std::string_view text);
    ERSLIB_EXPORT Result<std::string> sha256_hex_file(const fs::path& path);
}


// Exports ---------------------------------------------------------------------

namespace ers::crypto {
    using impl::sha256_hex;
    using impl::sha256_hex_file;
}
