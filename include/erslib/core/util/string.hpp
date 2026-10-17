#pragma once

// std
#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <span>
#include <string>

// ers
#include <erslib/core/stdlib/ctype.hpp>
#include <erslib/core/trait/string.hpp>
#include <erslib/core/type/fixed_string.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl::util {
    template<char... Args>
    constexpr std::string_view concat_chars() {
        static constexpr std::array<char, sizeof...(Args)> arr = { Args... };
        return std::string_view { arr, arr.size() };
    }


    template<fixed_string... Args>
	constexpr auto concat_literals() {
        fixed_string<(Args.strlen() + ...) + 1> result;
        size_t ptr = 0;

        auto append = [&](const auto& s) {
            for (const char it : s.to_sv())
                result.value[ptr++] = it;
        };

        (append(Args), ...);
        return result;
    }

    template<fixed_string... Args>
    inline constexpr auto concat_literals_v = concat_literals<Args...>();


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


namespace ers::impl {
    template<typename S, typename... Args>
    bool string_parts_compare(const S& s, size_t offset, Args&&... args) {
        if constexpr (sizeof...(args) == 0)
            return false;

        if (string_traits<S>::size(s) < (string_traits<Args>::size(args) + ...))
            return false;

        size_t i = offset;
        auto match = [&i, &s]<typename T>(const T& arg) {
            for (size_t j = 0; j < string_traits<T>::size(arg); i++, j++) {
                if (s[i] != string_traits<T>::index(arg, j))
                    return false;
            }

            return true;
        };

        return (match(args) && ...);
    }
}

namespace ers::impl::util {
    template<typename S, typename... Args>
    bool starts_with_seq(const S& s, Args&&... args) {
        return string_parts_compare(s, 0, std::forward<Args>(args)...);
    }

    template<typename S, typename... Args>
    bool ends_with_seq(const S& s, Args&&... args) {
        return string_parts_compare(s, string_traits<S>::size(s) - (string_traits<Args>::size(args) + ...), std::forward<Args>(args)...);
    }
}


namespace ers::impl::util {
    std::string ERSLIB_EXPORT replace(std::string_view where, std::string_view from, std::string_view to, size_t estimated_replacements = 4);
    std::string ERSLIB_EXPORT quote(std::string_view what);
    std::string ERSLIB_EXPORT bytes_to_string(std::span<const std::byte> bytes, std::string_view args);
}


namespace ers::impl::util {
    constexpr std::string_view trim_left(std::string_view text) noexcept {
        while (!text.empty() && is_space(text.front()))
            text.remove_prefix(1);

        return text;
    }

    constexpr std::string_view trim_right(std::string_view text) noexcept {
        while (!text.empty() && is_space(text.back()))
            text.remove_suffix(1);

        return text;
    }

    constexpr std::string_view trim(std::string_view text) noexcept {
        return trim_right(trim_left(text));
    }
}


namespace ers::impl::util {
    template<std::predicate<char> Pred>
    std::string keep_if(std::string_view text, Pred&& pred, size_t max_length = std::string::npos) {
        std::string out;
        out.reserve(std::min(text.size(), max_length));

        for (char c : text) {
            if (out.size() == max_length)
                break;

            if (pred(c))
                out.push_back(c);
        }

        return out;
    }
}


// Exports

namespace ers::util {
    using impl::util::concat_chars;
    
    using impl::util::concat_literals;
    using impl::util::concat_literals_v;
    
    using impl::util::concat_strings;


    using impl::util::starts_with_seq;
    using impl::util::ends_with_seq;


    using impl::util::replace;
    using impl::util::quote;


    using impl::util::trim_left;
    using impl::util::trim_right;
    using impl::util::trim;


    using impl::util::keep_if;
}
