#pragma once


namespace ers::internal {
    template<int>
    struct consteval_helper {};
}


namespace ers {
    template<typename F>
    consteval bool is_constexpr_friendly() {
        return requires {
            typename internal::consteval_helper<(F{}(), 1)>;
        };
    }

    template<typename F>
    static constexpr bool is_constexpr_friendly_v = is_constexpr_friendly<F>();
}
