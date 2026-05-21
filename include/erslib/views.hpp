#pragma once

// std
#include <ranges>

// ers
#include <erslib/adaptor/ranges.hpp>


namespace ers::views {
    template<typename MemPtr>
    auto mem_filter(MemPtr mem_ptr) {
        return adaptor::member_range(mem_ptr, []<typename T>(T&& range, auto&& fn) {
            return std::views::filter(std::forward<T>(range), fn);
        });
    }

    template<typename MemPtr>
    auto mem_transform(MemPtr mem_ptr) {
        return adaptor::member_range(mem_ptr, []<typename T>(T&& range, auto&& fn) {
            return std::views::transform(std::forward<T>(range), fn);
        });
    }
}
