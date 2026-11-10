#pragma once


namespace ers::impl {
    template<auto Value, auto... Options>
    inline constexpr bool is_one_of = ((Value == Options) || ...);
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::is_one_of;
}
