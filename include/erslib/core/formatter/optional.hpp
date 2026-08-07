#pragma once

// std
#include <format>
#include <optional>


template<typename T>
struct std::formatter<std::optional<T>> {
    // {:[open][close]}
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();

        if (it == ctx.end() || *it == '}')
            return it;

        if (*it != '}' && std::next(it) != ctx.end()) {
            _open = *it++;
            _close = *it++;
        }

        if (it != ctx.end() && *it != '}')
            throw std::format_error("Invalid range format spec");

        return it;
    }

    constexpr auto format(const std::optional<T>& v, std::format_context& ctx) const {
        auto out = ctx.out();
        *out++ = _open;
        *out++ = v ? std::format_to(out, "{}", *v) : std::format_to(out, "none");
        *out++ = _close;
        return out;
    }


private:
    char _open = '[';
    char _close = ']';
};
