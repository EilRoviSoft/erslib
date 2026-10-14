#pragma once

// std
#include <string>

// export
#include <erslib/export.hpp>


namespace ers::crypto::impl {
    struct argon2_params_t {
        size_t parallelism = 1;
        size_t memory_kib = 1 << 16;
        size_t iterations = 2;
    };


    std::string ERSLIB_EXPORT argon2_hash(std::string_view password, const argon2_params_t& params = {});
    bool ERSLIB_EXPORT argon2_verify(std::string_view password, std::string_view hash);
}

namespace ers::crypto {
    using impl::argon2_params_t;

    using impl::argon2_hash;
    using impl::argon2_verify;
}
