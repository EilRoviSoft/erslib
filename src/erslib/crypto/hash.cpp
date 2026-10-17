#include "erslib/crypto/hash.hpp"

// std
#include <array>
#include <fstream>

// botan
#include <botan/hash.h>
#include <botan/hex.h>


// Details ---------------------------------------------------------------------

namespace {
    constexpr size_t chunk_size = 1 << 16;
}


// Implementation --------------------------------------------------------------

std::string ers::crypto::impl::sha256_hex(std::span<const std::byte> bytes) {
    auto hasher = Botan::HashFunction::create_or_throw("SHA-256");
    hasher->update(reinterpret_cast<const uint8_t*>(bytes.data()), bytes.size());

    return Botan::hex_encode(hasher->final(), false);
}

std::string ers::crypto::impl::sha256_hex(std::string_view text) {
    return sha256_hex(std::as_bytes(std::span(text)));
}

ers::Result<std::string> ers::crypto::impl::sha256_hex_file(const fs::path& path) {
    auto stream = std::ifstream(path, std::ios::binary);

    if (!stream)
        return make_error("Cannot read '{}'.", path.generic_string());

    auto hasher = Botan::HashFunction::create_or_throw("SHA-256");
    auto buffer = std::array<char, chunk_size>();

    while (stream) {
        stream.read(buffer.data(), buffer.size());

        if (auto got = stream.gcount(); got > 0)
            hasher->update(reinterpret_cast<const uint8_t*>(buffer.data()), static_cast<size_t>(got));
    }

    if (stream.bad())
        return make_error("Read of '{}' failed.", path.generic_string());

    return Botan::hex_encode(hasher->final(), false);
}
