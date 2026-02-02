#pragma once

// std
#include <bitset>

namespace ers::util {
    template<typename T>
    concept BitIndex = std::unsigned_integral<T> || std::is_enum_v<T>;

    template<size_t TSize, BitIndex... TIndexes>
    constexpr std::bitset<TSize> from_numbers(TIndexes... indexes) {
        std::bitset<TSize> result;
        ((result.set(static_cast<size_t>(indexes))), ...);
        return result;
    }
}
