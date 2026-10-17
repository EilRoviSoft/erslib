#include "erslib/crypto/random.hpp"

// botan
#include <botan/auto_rng.h>
#include <botan/hex.h>


// Details ---------------------------------------------------------------------

namespace {
    Botan::RandomNumberGenerator& rng() {
        thread_local Botan::AutoSeeded_RNG instance;
        return instance;
    }
}


// Implementation --------------------------------------------------------------

std::vector<std::byte> ers::crypto::impl::random_bytes(size_t count) {
    std::vector<std::byte> out(count);
    rng().randomize(reinterpret_cast<uint8_t*>(out.data()), out.size());

    return out;
}

std::string ers::crypto::impl::random_hex(size_t byte_count) {
    auto bytes = random_bytes(byte_count);
    return Botan::hex_encode(reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size(), false);
}
