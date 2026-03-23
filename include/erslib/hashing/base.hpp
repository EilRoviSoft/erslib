#pragma once

// std
#include <array>
#include <bit>
#include <span>
#include <string>


// Definition

namespace ers::hashing {
    template<typename Policy>
    struct backend {};

    template<typename Policy>
    concept RawBytesBackend = requires(std::span<const std::byte> what, size_t seed) {
        { backend<Policy>::process_raw_bytes(what, seed) } -> std::same_as<size_t>;
    };
}


namespace ers {
    template<typename T, typename Policy>
    struct THashBase {};
}


// Specializations for strings

template<typename Char, size_t N, typename Policy>
    requires std::same_as<std::remove_cvref_t<Char>, char>
struct ers::THashBase<std::array<Char, N>, Policy> {
    using type = std::array<Char, N>;

    constexpr size_t operator()(
        const type& what, size_t seed = 0
    ) const noexcept requires (hashing::RawBytesBackend<Policy>) {
        std::array<std::byte, what.size()> bytes = {};


        for (size_t i = 0; i < what.size(); i++)
            bytes[i] = static_cast<std::byte>(what[i]);


        return hashing::backend<Policy>::process_raw_bytes({ bytes.data(), what.size() }, seed);
    }

    constexpr size_t operator()(
        const type& what, size_t seed = 0
    ) const noexcept requires (!hashing::RawBytesBackend<Policy>) {
        return hashing::backend<Policy>::process_value(what, seed);
    }
};


template<size_t N, typename Policy>
struct ers::THashBase<const char[N], Policy> {
    using type = const char[N];

    constexpr size_t operator()(const type& what, size_t seed = 0) const noexcept {
        return THashBase<const std::array<const char, N>, Policy> {}(std::bit_cast<std::array<const char, N>>(what), seed);
    }
};

template<typename Policy>
struct ers::THashBase<const char*, Policy> {
    using type = const char*;

    size_t operator()(
        type what, size_t seed = 0
    ) const noexcept requires (hashing::RawBytesBackend<Policy>) {
        const auto bytes = reinterpret_cast<const std::byte*>(what);
        return hashing::backend<Policy>::process_raw_bytes({ bytes, std::strlen(what) }, seed);
    }

    size_t operator()(
        type what, size_t seed = 0
    ) const noexcept requires (!hashing::RawBytesBackend<Policy>) {
        return hashing::backend<Policy>::process_value(what, seed);
    }
};

template<typename Policy>
struct ers::THashBase<std::string_view, Policy> {
    using type = std::string_view;

    size_t operator()(
        type what, size_t seed = 0
    ) const noexcept requires (hashing::RawBytesBackend<Policy>) {
        const auto bytes = reinterpret_cast<const std::byte*>(what.data());
        return hashing::backend<Policy>::process_raw_bytes({ bytes, what.size() }, seed);
    }

    size_t operator()(
        type what, size_t seed = 0
    ) const noexcept requires (!hashing::RawBytesBackend<Policy>) {
        return hashing::backend<Policy>::process_value(what, seed);
    }
};

template<typename Policy>
struct ers::THashBase<std::string, Policy> {
    using type = std::string;

    size_t operator()(
        const type& what, size_t seed = 0
    ) const requires (hashing::RawBytesBackend<Policy>) {
        const auto bytes = reinterpret_cast<const std::byte*>(what.data());
        return hashing::backend<Policy>::process_raw_bytes({ bytes, what.size() }, seed);
    }

    size_t operator()(
        const type& what, size_t seed = 0
    ) const requires (!hashing::RawBytesBackend<Policy>) {
        return hashing::backend<Policy>::process_value(what, seed);
    }
};


// Specializations for numeric types (including enums)

template<std::integral T, typename Policy>
struct ers::THashBase<T, Policy> {
    constexpr size_t operator()(
        T what, size_t seed = 0
    ) const noexcept requires (hashing::RawBytesBackend<Policy>) {
        constexpr size_t size = sizeof(T);
        const auto bytes = std::bit_cast<std::array<std::byte, size>>(what);
        return hashing::backend<Policy>::process_raw_bytes({ bytes.data(), size }, seed);
    }

    constexpr size_t operator()(
        T what, size_t seed = 0
    ) const noexcept requires (!hashing::RawBytesBackend<Policy>) {
        return hashing::backend<Policy>::process_value(what, seed);
    }
};


template<typename T, typename Policy>
    requires (std::is_enum_v<T> || std::is_scoped_enum_v<T>)
struct ers::THashBase<T, Policy> {
    constexpr size_t operator()(T what, size_t seed) const noexcept {
        return ers::THashBase<std::underlying_type_t<T>, Policy> {}(std::to_underlying(what), seed);
    }
};
