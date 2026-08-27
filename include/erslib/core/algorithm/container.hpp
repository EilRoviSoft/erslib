#pragma once

// std
#include <algorithm>
#include <ranges>
#include <vector>

// ers
#include <erslib/core/adaptor/transparent_base.hpp>
#include <erslib/core/concept/pair.hpp>


namespace ers::impl::algo {
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

namespace ers::impl::algo {
    template<auto Member, typename T, typename Alloc, typename Compare = std::less<>>
    typename std::vector<T, Alloc>::iterator insert_sorted(std::vector<T, Alloc>& v, T value, Compare comp = {}) {
        const auto pos = std::ranges::upper_bound(
            v, project<Member>(value), comp,
            [](const T& e) -> decltype(auto) { return project<Member>(e); }
        );

        return v.insert(pos, std::move(value));
    }
}
