#pragma once

// std
#include <cstddef>
#include <span>
#include <string>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace ers::crypto::impl {
    std::string ERSLIB_EXPORT sha256_hex(std::span<const std::byte> bytes);
    std::string ERSLIB_EXPORT sha256_hex(std::string_view text);
    Result<std::string> ERSLIB_EXPORT sha256_hex_file(const fs::path& path);
}


// Exports

namespace ers::crypto {
    using impl::sha256_hex;
    using impl::sha256_hex_file;
}
