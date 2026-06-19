#pragma once

// std
#include <algorithm>
#include <ranges>
#include <vector>

// ers
#include <erslib/core/concept/pair.hpp>


namespace ers::algo {
    template<std::ranges::range R, typename OutIt, typename Compare = std::less<>>
        requires PairLike<std::ranges::range_reference_t<R>>
    void keys_sorted_by_value(const R& range, OutIt out) {
        using iter_t = std::ranges::iterator_t<const R>;

        std::vector<iter_t> sorted_items;
        sorted_items.reserve(std::ranges::distance(range));

        for (auto it = std::ranges::begin(range); it != std::ranges::end(range); it++)
            sorted_items.emplace_back(it);

        std::ranges::sort(sorted_items, Compare{}, [](auto it) { return it->second; });

        for (auto it : sorted_items)
            *out++ = it->first;
    }
}
