#pragma once

// std
#include <string>

// ers
#include <erslib/hashing/std.hpp>
#include <erslib/trait/string.hpp>

// Common string utils for hashing, comparing and allocation

namespace ers::util {
    template<template<typename> typename THashEngine = hashing::Std>
    struct string_hash {
        using is_transparent = void;

        constexpr size_t operator()(const std::string_view sv) const {
            return THashEngine<std::string_view> {}(sv);
        }
        constexpr size_t operator()(const std::string& str) const {
            return THashEngine<std::string> {}(str);
        }
        constexpr size_t operator()(const char* cstr) const {
            return THashEngine<std::string_view> {}(cstr);
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

// concrete functions

namespace ers::util {
    template<char... TChars>
    constexpr std::string_view concat_chars() {
        static constexpr char arr[] = { TChars..., '\0' };
        return std::string_view { arr, sizeof...(TChars) };
    }

    template<typename... TArgs>
    std::string concat_strings(TArgs&&... args) {
        std::string result;

        if constexpr (sizeof...(args) == 0)
            return result;

        result.reserve((string_traits<TArgs>::size(args) + ...));
        (string_traits<TArgs>::append(result, std::forward<TArgs>(args)), ...);

        return result;
    }
}
