#pragma once

// std
#include <array>
#include <string>

// ers
#include <erslib/trait/string.hpp>
#include <erslib/type/fixed_string.hpp>

// export
#include <erslib/export.hpp>


namespace ers::util {
    template<char... Args>
    constexpr std::string_view concat_chars() {
        static constexpr std::array<char, sizeof...(Args)> arr = { Args... };
        return std::string_view { arr, arr.size() };
    }

    template<fixed_string... Args>
	constexpr auto concat_literals() {
        fixed_string<(Args.size() + ...)> result;
        size_t ptr = 0;


        auto append = [&](const auto& s) {
            for (size_t i = 0; i < s.size(); i++)
                result.value[ptr + i] = s.value[i];
            ptr += s.size();
        };

        (append(Args), ...);


        return result;
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


namespace ers::internal {
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

namespace ers::util {
    template<typename S, typename... Args>
    bool starts_with_seq(const S& s, Args&&... args) {
        return internal::string_parts_compare(s, 0, std::forward<Args>(args)...);
    }

    template<typename S, typename... Args>
    bool ends_with_seq(const S& s, Args&&... args) {
        return internal::string_parts_compare(s, string_traits<S>::size(s) - (string_traits<Args>::size(args) + ...), std::forward<Args>(args)...);
    }
}


namespace ers::util {
    std::string ERSLIB_EXPORT replace(std::string_view where, std::string_view from, std::string_view to, size_t estimated_replacements = 4);
}
