#pragma once


namespace ers::impl {
    template<class... Ts>
    struct overloads : Ts... {
        using Ts::operator()...;
    };
}


// Exports

namespace ers {
    using impl::overloads;
}
