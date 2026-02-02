#pragma once

// std
#include <concepts>

namespace ers::util {
    // v ∈ (a, b)
    template<std::integral Tv, std::integral Ta, std::integral Tb>
    bool between(Tv v, Ta a, Tb b) {
        return v > a && v < b;
    }
    // v ∈ [a, b)
    template<std::integral Tv, std::integral Ta, std::integral Tb>
    bool between_li(Tv v, Ta a, Tb b) {
        return v >= a && v < b;
    }
    // v ∈ (a, b]
    template<std::integral Tv, std::integral Ta, std::integral Tb>
    bool between_ri(Tv v, Ta a, Tb b) {
        return v > a && v <= b;
    }
    // v ∈ [a, b]
    template<std::integral Tv, std::integral Ta, std::integral Tb>
    bool between_lr(Tv v, Ta a, Tb b) {
        return v >= a && v <= b;
    }
}
