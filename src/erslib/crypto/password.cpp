#include "erslib/crypto/password.hpp"

// botan
#include <botan/argon2fmt.h>
#include <botan/auto_rng.h>


std::string ers::crypto::impl::argon2_hash(std::string_view password, const argon2_params_t& params) {
    auto rng = Botan::AutoSeeded_RNG();

    return Botan::argon2_generate_pwhash(
        password.data(),
        password.size(),
        rng,
        params.parallelism,
        params.memory_kib,
        params.iterations
    );
}

bool ers::crypto::impl::argon2_verify(std::string_view password, std::string_view hash) {
    return Botan::argon2_check_pwhash(password.data(), password.size(), hash);
}
