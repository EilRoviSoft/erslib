#pragma once

// std
#include <concepts>


namespace ers::impl::util {
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


// Exports

namespace ers {
    using impl::util::between;
    using impl::util::between_li;
    using impl::util::between_ri;
    using impl::util::between_lr;
}
