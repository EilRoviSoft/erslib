#pragma once

// std
#include <cstddef>
#include <string>
#include <vector>

// export
#include <erslib/export.hpp>


namespace ers::crypto::impl {
    std::vector<std::byte> ERSLIB_EXPORT random_bytes(size_t count);
    std::string ERSLIB_EXPORT random_hex(size_t byte_count);
}

namespace ers::crypto {
    using impl::random_bytes;
    using impl::random_hex;
}
