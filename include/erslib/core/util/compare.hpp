#pragma once

// std
#include <concepts>


namespace ers::util {
    // v ∈ (a, b)
    template<std::integral V, std::integral A, std::integral B>
    bool between(V v, A a, B b) {
        return v > a && v < b;
    }
    // v ∈ [a, b)
    template<std::integral V, std::integral A, std::integral B>
    bool between_li(V v, A a, B b) {
        return v >= a && v < b;
    }
    // v ∈ (a, b]
    template<std::integral V, std::integral A, std::integral B>
    bool between_ri(V v, A a, B b) {
        return v > a && v <= b;
    }
    // v ∈ [a, b]
    template<std::integral V, std::integral A, std::integral B>
    bool between_lr(V v, A a, B b) {
        return v >= a && v <= b;
    }
}
