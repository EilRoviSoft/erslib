#pragma once


namespace ers::impl {
    template<typename T>
    concept PairLike = requires(const T& v) {
        v.first;
        v.second;
    };
}
