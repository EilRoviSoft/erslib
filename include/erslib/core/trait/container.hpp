#pragma once

// std
#include <utility>


namespace ers::impl {
    template<typename T>
    concept HintInsertable = requires(T c, typename T::value_type v) {
        c.insert(c.end(), std::move(v));
    };

    template<typename T>
    concept Reservable = requires(T c) {
        c.reserve(0uz);
    };
}


// Exports

namespace ers {
    using impl::HintInsertable;
    using impl::Reservable;
}
