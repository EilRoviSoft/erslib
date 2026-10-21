#pragma once

// std
#include <ranges>
#include <utility>
#include <vector>

// ers
#include <erslib/core/adaptor/ranges.hpp>


namespace ers::impl::views {
    template<typename MemPtr>
    auto mem_filter(MemPtr mem_ptr) {
        return member_range(mem_ptr, []<typename T>(T&& range, auto&& fn) {
            return std::views::filter(std::forward<T>(range), fn);
        });
    }

    template<typename MemPtr>
    auto mem_transform(MemPtr mem_ptr) {
        return member_range(mem_ptr, []<typename T>(T&& range, auto&& fn) {
            return std::views::transform(std::forward<T>(range), fn);
        });
    }
}

namespace ers::impl::ranges {
    template<std::ranges::input_range R, typename Proj>
    auto collect(R&& range, Proj proj) {
        return std::forward<R>(range)
            | std::views::transform(proj)
            | std::ranges::to<std::vector>();
    }
}
