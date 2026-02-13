#pragma once

// std
#include <array>
#include <string>

// ers
#include <erslib/concept/string.hpp>
#include <erslib/trait/string.hpp>
#include <erslib/type/fixed_string.hpp>

// Common string utils for hashing, comparing and allocation

namespace ers::util {
    template<template<typename> typename HashEngine>
    struct string_hash_adaptor {
        using is_transparent = void;

        constexpr size_t operator()(const std::string_view sv) const {
            return HashEngine<std::string_view> {}(sv);
        }
        constexpr size_t operator()(const std::string& str) const {
            return HashEngine<std::string> {}(str);
        }
        constexpr size_t operator()(const char* cstr) const {
            return HashEngine<const char*> {}(cstr);
        }
    };

    struct string_equal {
        using is_transparent = void;

        constexpr bool operator()(const std::string& a, const std::string& b) const noexcept {
            return a == b;
        }

        constexpr bool operator()(const std::string_view a, const std::string_view b) const noexcept {
            return a == b;
        }
        constexpr bool operator()(const std::string& a, const std::string_view b) const noexcept {
            return a == b;
        }
        constexpr bool operator()(const std::string_view a, const std::string& b) const noexcept {
            return a == b;
        }

        constexpr bool operator()(const std::string& a, const char* b) const noexcept {
            return a == b;
        }
        constexpr bool operator()(const char* a, const std::string& b) const noexcept {
            return a == b;
        }
    };
}

// Implementation

namespace ers::internal {
	template<fixed_string... Args>
	constexpr auto make_literals_array() {
        constexpr size_t size = (Args.size() + ...);
        std::array<char, size> result = {};

        size_t ptr = 0;

        auto append = [&](const auto& s) {
            for (size_t i = 0; i < s.size(); i++)
                result[ptr + i] = s.value[i];
            ptr += s.size();
        };

        (append(Args), ...);

        return result;
	}
}

// Concrete functions

namespace ers::util {
    template<char... Args>
    constexpr std::string_view concat_chars() {
        static constexpr std::array<char, sizeof...(Args)> arr = { Args... };
        return std::string_view { arr, arr.size() };
    }

    template<fixed_string... Args>
	constexpr std::string_view concat_literals() {
        static constexpr auto arr = internal::make_literals_array<Args...>();
        return std::string_view { arr.data(), arr.size() };
    }

    template<typename... Args>
    std::string concat_strings(Args&&... args) {
        std::string result;

        if constexpr (sizeof...(args) == 0)
            return result;

        result.reserve((string_traits<Args>::size(args) + ...));
        (string_traits<Args>::append(result, std::forward<Args>(args)), ...);

        return result;
    }
}
