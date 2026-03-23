#pragma once

// std
#include <array>
#include <string>

// ers
#include <erslib/trait/string.hpp>
#include <erslib/type/fixed_string.hpp>


// Concrete functions

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
