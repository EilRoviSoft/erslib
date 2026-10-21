#pragma once

// std
#include <string>
#include <string_view>

// export
#include <erslib/export.hpp>


namespace ers::crypto::impl {
    struct argon2_params_t {
        size_t parallelism = 1;
        size_t memory_kib = 1 << 16;
        size_t iterations = 2;
    };


    ERSLIB_EXPORT std::string argon2_hash(std::string_view password, const argon2_params_t& params = {});
    ERSLIB_EXPORT bool argon2_verify(std::string_view password, std::string_view hash);
}

namespace ers::crypto {
    using impl::argon2_params_t;

    using impl::argon2_hash;
    using impl::argon2_verify;
}
