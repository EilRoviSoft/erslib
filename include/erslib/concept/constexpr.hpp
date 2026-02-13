#pragma once


namespace ers::internal {
    template<int>
    struct consteval_helper {};
}


namespace ers {
    template<typename F>
    consteval bool is_consteval() {
        return requires {
            typename internal::consteval_helper<(F{}(), 1)>;
        };
    }
}
