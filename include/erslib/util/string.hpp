#pragma once

// std
#include <string>

// ers
#include <erslib/trait/stringly.hpp>

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

        result.reserve((trait::get_size(args) + ...));
        (trait::append_to_string(result, std::forward<TArgs>(args)), ...);

        return result;
    }
}
