#pragma once

// std
#include <string>


namespace ers {
    template<typename T>
    constexpr std::string_view to_sv(const T& what) = delete("default specialization");

    template<typename T>
    std::string to_string(const T& what) = delete("default specialization");
}


namespace ers {
    template<typename T>
    concept HasStdToString = requires(const T& t) {
        { std::to_string(t) } -> std::convertible_to<std::string>;
    };

    template<typename T>
        requires HasStdToString<T>
    std::string to_string(const T& what) {
        return std::to_string(what);
    }
}
