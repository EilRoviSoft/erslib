#pragma once

// std
#include <string>


// Definition

namespace ers {
    template<typename T>
    struct to_sv {
        constexpr std::string_view operator()(const T& what) const noexcept = delete("default specialization");
    };

    template<typename T>
    struct to_str {
        std::string operator()(const T& what) const = delete("default specialization");
    };
}


// Implementation for std conversions

namespace ers {
    template<typename T>
    concept HasStdToString = requires(const T& t) {
        { std::to_string(t) } -> std::convertible_to<std::string>;
    };

    template<HasStdToString T>
    struct to_str<T> {
        std::string operator()(const T& what) const {
            return std::to_string(what);
        }
    };
}
