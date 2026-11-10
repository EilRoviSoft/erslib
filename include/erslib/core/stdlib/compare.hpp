#pragma once

// std
#include <concepts>
#include <utility>


namespace ers::impl {
    // v ∈ (a, b)
    template<std::integral V, std::integral A, std::integral B>
    constexpr bool is_between(V v, A a, B b) {
        return std::cmp_greater(v, a) && std::cmp_less(v, b);
    }
    // v ∈ (a, b]
    template<std::integral V, std::integral A, std::integral B>
    constexpr bool is_between_upto(V v, A a, B b) {
        return std::cmp_greater(v, a) && std::cmp_less_equal(v, b);
    }
    // v ∈ [a, b)
    template<std::integral V, std::integral A, std::integral B>
    constexpr bool is_between_from(V v, A a, B b) {
        return std::cmp_greater_equal(v, a) && std::cmp_less(v, b);
    }
    // v ∈ [a, b]
    template<std::integral V, std::integral A, std::integral B>
    constexpr bool is_in_range(V v, A a, B b) {
        return std::cmp_greater_equal(v, a) && std::cmp_less_equal(v, b);
    }
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::is_between;
    using impl::is_between_upto;
    using impl::is_between_from;
    using impl::is_in_range;
}
