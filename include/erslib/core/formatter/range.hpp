#pragma once

// std
#include <concepts>
#include <format>
#include <ranges>
#include <string_view>


template<std::ranges::range R>
    requires (!std::convertible_to<R, std::string_view>)
struct std::formatter<R> {
    // {:[open][close][sep]}
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();

        if (it == ctx.end() || *it == '}')
            return it;

        if (*it != '}' && std::next(it) != ctx.end()) {
            _open = *it++;
            _close = *it++;
        }

        if (it != ctx.end() && *it != '}')
            _separator = *it++;

        if (it != ctx.end() && *it != '}')
            throw std::format_error("Invalid range format spec");

        return it;
    }


    auto format(const R& range, std::format_context& ctx) const {
        auto out = ctx.out();
        *out++ = _open;

        bool first = true;
        for (const auto& elem : range) {
            if (!first) {
                *out++ = _separator;
                *out++ = ' ';
            }

            out = std::format_to(out, "{}", elem);
            first = false;
        }

        *out++ = _close;
        return out;
    }


private:
    char _open = '[';
    char _close = ']';
    char _separator = ',';
};
