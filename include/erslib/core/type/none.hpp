#pragma once


namespace ers::impl {
    struct none_t {};

    static constexpr none_t none;
}


// Exports

namespace ers {
    using impl::none_t;
    using impl::none;
}
