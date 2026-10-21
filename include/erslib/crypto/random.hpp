#pragma once

// std
#include <cstddef>
#include <string>
#include <vector>

// export
#include <erslib/export.hpp>


namespace ers::crypto::impl {
    ERSLIB_EXPORT std::vector<std::byte> random_bytes(size_t count);
    ERSLIB_EXPORT std::string random_hex(size_t byte_count);
}

namespace ers::crypto {
    using impl::random_bytes;
    using impl::random_hex;
}
