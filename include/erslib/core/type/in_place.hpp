#pragma once


namespace ers::impl {
    template<typename... Ts>
    struct in_place_types_t {};

    template<typename... Ts>
    inline constexpr in_place_types_t<Ts...> in_place_types {};
}


// Exports

namespace ers {
    using impl::in_place_types_t;
    using impl::in_place_types;
}
