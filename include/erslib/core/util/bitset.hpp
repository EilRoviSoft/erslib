#pragma once

// std
#include <bitset>


namespace ers::util {
    template<typename T>
    concept BitIndex = std::unsigned_integral<T> || std::is_enum_v<T>;

    template<size_t S, BitIndex... Indexes>
    constexpr std::bitset<S> from_numbers(Indexes... indexes) {
        std::bitset<S> result;
        ((result.set(static_cast<size_t>(indexes))), ...);
        return result;
    }
}
