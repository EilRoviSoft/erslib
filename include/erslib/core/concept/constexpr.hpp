#pragma once


namespace ers::impl {
    template<int>
    struct consteval_helper {};
}


namespace ers::impl {
    template<typename F>
    consteval bool is_constexpr_friendly() {
        return requires {
            typename consteval_helper<(F{}(), 1)>;
        };
    }

    template<typename F>
    static constexpr bool is_constexpr_friendly_v = is_constexpr_friendly<F>();
}
