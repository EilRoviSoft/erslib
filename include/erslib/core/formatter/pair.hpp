#pragma once

// std
#include <format>
#include <ranges>


template<typename K, typename V>
struct std::formatter<std::pair<K, V>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const std::pair<K, V>& p, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "({}, {})", p.first, p.second);
    }
};
