#pragma once

// std
#include <string>
#include <string_view>

// ers
#include <erslib/core/stdlib/compare.hpp>


// Char implementation

namespace ers::impl {
    constexpr bool is_digit(char c) noexcept {
        return is_in_range(c, '0', '9');
    }

    constexpr bool is_lower(char c) noexcept {
        return is_in_range(c, 'a', 'z');
    }

    constexpr bool is_upper(char c) noexcept {
        return is_in_range(c, 'A', 'Z');
    }

    constexpr bool is_alpha(char c) noexcept {
        return is_in_range(c | 0x20, 'a', 'z');
    }

    constexpr bool is_alnum(char c) noexcept {
        return is_alpha(c) || is_digit(c);
    }

    constexpr bool is_xdigit(char c) noexcept {
        return is_digit(c) || is_in_range(c | 0x20, 'a', 'f');
    }

    constexpr bool is_cntrl(char c) noexcept {
        return c < 0x20 || c == 0x7F;
    }

    constexpr bool is_graph(char c) noexcept {
        return is_in_range(c, 0x21, 0x5E);
    }

    constexpr bool is_print(char c) noexcept {
        return is_in_range(c, 0x20, 0x5F);
    }

    constexpr bool is_space(char c) noexcept {
        return c == ' ' || is_in_range(c, '\t', '\t' + 5);
    }

    constexpr bool is_blank(char c) noexcept {
        return c == ' ' || c == '\t';
    }

    constexpr bool is_punct(char c) noexcept {
        return is_graph(c) && !is_alnum(c);
    }
}

namespace ers::impl {
    constexpr char to_lower(char c) noexcept {
        return c | (is_upper(c) ? 0x20 : 0);
    }

    constexpr char to_upper(char c) noexcept {
        return c & ~(is_lower(c) ? 0x20 : 0);
    }
}


// String implementation

namespace ers::impl {
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


// Export

namespace ers {
    using impl::is_digit;
    using impl::is_lower;
    using impl::is_upper;
    using impl::is_alpha;
    using impl::is_alnum;
    using impl::is_xdigit;
    using impl::is_cntrl;
    using impl::is_graph;
    using impl::is_print;
    using impl::is_space;
    using impl::is_blank;
    using impl::is_punct;

    using impl::to_lower;
    using impl::to_upper;
    using impl::capitalise_first;
}
