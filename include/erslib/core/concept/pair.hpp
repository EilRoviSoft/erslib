#pragma once


namespace ers {
    template<typename T>
    concept PairLike = requires(const T& v) {
        v.first;
        v.second;
    };
}
