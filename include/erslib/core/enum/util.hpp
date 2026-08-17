#pragma once

// std
#include <string>
#include <string_view>


namespace ers::impl {
    constexpr char to_lower(char c) noexcept {
        return (c >= 'A' && c <= 'Z') ? static_cast<char>(c + ('a' - 'A')) : c;
    }

    constexpr char to_upper(char c) noexcept {
        return (c >= 'a' && c <= 'z') ? static_cast<char>(c - ('a' - 'A')) : c;
    }

    constexpr bool is_lower(char c) noexcept {
        return c >= 'a' && c <= 'z';
    }

    constexpr bool is_upper(char c) noexcept {
        return c >= 'A' && c <= 'Z';
    }

    constexpr bool is_digit(char c) noexcept {
        return c >= '0' && c <= '9';
    }


    constexpr std::string to_lower(std::string_view s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
            out += to_lower(c);
        return out;
    }

    constexpr std::string to_upper(std::string_view s) {
        std::string out;
        out.reserve(s.size());
        for (char c : s)
            out += to_upper(c);
        return out;
    }

    constexpr std::string capitalise_first(std::string_view s) {
        std::string out(s);
        if (!out.empty())
            out[0] = to_upper(out[0]);
        return out;
    }
}
