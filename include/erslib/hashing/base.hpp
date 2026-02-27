#pragma once

// std
#include <array>
#include <bit>
#include <span>
#include <string>

// ers
#include <erslib/concept/constexpr.hpp>


// Definition

namespace ers::hashing {
    template<typename Policy>
    struct backend {};


    template<typename Policy>
    constexpr bool is_backend_handles_raw_bytes() {
        return requires(std::span<const std::byte> what, size_t seed) { backend<Policy>::process_raw_bytes(what, seed); };
    }

    template<typename Policy>
    static constexpr bool is_backend_handles_raw_bytes_v = is_backend_handles_raw_bytes<Policy>();


    template<typename Policy>
    constexpr bool is_constexpr_raw_hash() {
        return ers::is_constexpr_friendly_v<typename backend<Policy>::process_raw_bytes>;
    }

    template<typename Policy>
    static constexpr bool is_constexpr_raw_hash_v = is_constexpr_raw_hash<Policy>();
}


namespace ers {
    template<typename T, typename Policy>
    struct THashBase {};
}


// Specializations

template<size_t N, typename Policy>
struct ers::THashBase<const std::array<char, N>, Policy> {
    using type = const std::array<char, N>&;

    constexpr size_t operator()(type what, size_t seed = 0) const noexcept
        requires (hashing::is_backend_handles_raw_bytes_v<Policy>) {
        std::array<std::byte, what.size()> bytes = {};

        for (size_t i = 0; i < what.size(); i++)
            bytes[i] = static_cast<std::byte>(what[i]);

        return hashing::backend<Policy>::process_raw_bytes({ bytes.data(), what.size() }, seed);
    }

    constexpr size_t operator()(type what, size_t seed = 0) const noexcept
        requires (!hashing::is_backend_handles_raw_bytes_v<Policy>) {
        return hashing::backend<Policy>::process_raw_bytes(what, seed);
    }
};

template<size_t N, typename Policy>
struct ers::THashBase<const char[N], Policy> {
    using type = const char(&)[N];

    constexpr size_t operator()(type what, size_t seed = 0) const noexcept {
        return THashBase<std::array<const char, N>, Policy> {}(std::bit_cast<const char(&)[N]>(what), seed);
    }
};

template<typename Policy>
struct ers::THashBase<std::string_view, Policy> {
    using type = std::string_view;

    size_t operator()(type what, size_t seed = 0) const noexcept
        requires (hashing::is_backend_handles_raw_bytes_v<Policy>) {
        const auto bytes = reinterpret_cast<const std::byte*>(what.data());
        return hashing::backend<Policy>::process_raw_bytes({ bytes, what.size() }, seed);
    }

    size_t operator()(type what, size_t seed = 0) const noexcept
        requires (!hashing::is_backend_handles_raw_bytes_v<Policy>) {
        return hashing::backend<Policy>::process_raw_bytes(what, seed);
    }
};

template<typename Policy>
struct ers::THashBase<std::string, Policy> {
    using type = const std::string&;

    size_t operator()(type what, size_t seed = 0) const
        requires (hashing::is_backend_handles_raw_bytes_v<Policy>) {
        const auto bytes = reinterpret_cast<const std::byte*>(what.data());
        return hashing::backend<Policy>::process_raw_bytes({ bytes, what.size() }, seed);
    }

    size_t operator()(type what, size_t seed = 0) const
        requires (!hashing::is_backend_handles_raw_bytes_v<Policy>) {
        return hashing::backend<Policy>::process_raw_bytes(what, seed);
    }
};


template<std::integral T, typename Policy>
struct ers::THashBase<T, Policy> {
    constexpr size_t operator()(T what, size_t seed = 0) const noexcept
        requires (hashing::is_backend_handles_raw_bytes_v<Policy>) {
        constexpr size_t size = sizeof(T);
        const auto bytes = std::bit_cast<std::array<std::byte, size>>(what);
        return hashing::backend<Policy>::process_raw_bytes({ bytes.data(), size }, seed);
    }

    constexpr size_t operator()(T what, size_t seed = 0) const noexcept
        requires (!hashing::is_backend_handles_raw_bytes_v<Policy>) {
        return hashing::backend<Policy>::process_value(what, seed);
    }
};
