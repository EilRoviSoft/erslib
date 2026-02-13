#pragma once

// std
#include <bit>
#include <string_view>

#include "base.hpp"

namespace ers::internal {
    // To work properly, you should override only 1 per algo
    template<typename Policy>
    struct backend {
        static constexpr size_t process(
            const std::byte* data,
            size_t size,
            size_t seed
        ) noexcept = delete("non-specialized definition");

        template<typename T>
        static constexpr size_t process_value(
            const T& data,
            size_t seed
        ) noexcept = delete("non-specialized definition");
    };
}

namespace ers {
    template<typename T, typename Policy>
    struct THashBase {
        constexpr size_t operator()(const T& what, size_t seed = 0) const noexcept = delete("non-specialized definition");
    };
}

template<std::integral T, typename Policy>
struct ers::THashBase<T, Policy> {
    constexpr size_t operator()(T what, size_t seed = 0) const noexcept {
        if constexpr (requires { internal::backend<Policy>::process(what, seed); })
            return internal::backend<Policy>::process_value(what, seed);
        else
            return internal::backend<Policy>::process(std::bit_cast<const std::byte>(what), sizeof(T), seed);
    }
};

template<const char* V, typename Policy>
struct ers::THashBase<const char*, Policy> {
    constexpr size_t operator()(const std::string_view what, size_t seed = 0) const noexcept {
        if constexpr (requires { internal::backend<Policy>::process(what, seed); })
            return internal::backend<Policy>::process_value(what, seed);
        else
            return internal::backend<Policy>::process(std::bit_cast<const std::byte>(what.data()), what.size(), seed);
    }
};

template<typename Policy>
struct ers::THashBase<std::string_view, Policy> {
    size_t operator()(const std::string_view what, size_t seed = 0) const noexcept {
        if constexpr (requires { internal::backend<Policy>::process(what, seed); })
            return internal::backend<Policy>::process_value(what, seed);
        else
            return internal::backend<Policy>::process(reinterpret_cast<const std::byte*>(std::bit_cast<const std::byte>()), what.size(), seed);
    }
};
