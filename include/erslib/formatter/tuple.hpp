#pragma once

// std
#include <format>
#include <ranges>
#include <tuple>


template<typename... Ts>
struct std::formatter<std::tuple<Ts...>> {
public:
    // {:[open][close][sep]}
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();

        if (it == ctx.end() || *it == '}')
            return it;

        if (std::next(it) != ctx.end() && *std::next(it) != '}') {
            _open = *it++;
            _close = *it++;
        }

        if (it != ctx.end() && *it != '}')
            _separator = *it++;

        if (it != ctx.end() && *it != '}')
            throw std::format_error("Invalid tuple format spec");

        return it;
    }

    auto format(const std::tuple<Ts...>& t, std::format_context& ctx) const {
        auto out = ctx.out();
        *out++ = _open;

        if constexpr (sizeof...(Ts) > 0)
            out = format_all(t, ctx, std::index_sequence_for<Ts...> {});

        *out++ = _close;
        return out;
    }


private:
    char _open = '(';
    char _close = ')';
    char _separator = ',';


    template<std::size_t I>
    auto format_element(
        const std::tuple<Ts...>& t,
        std::format_context::iterator out
    ) const {
        if constexpr (I > 0) {
            *out++ = _separator;
            *out++ = ' ';
        }
        return std::format_to(out, "{}", std::get<I>(t));
    }

    template<std::size_t... Is>
    auto format_all(
        const std::tuple<Ts...>& t,
        std::format_context& ctx,
        std::index_sequence<Is...>
    ) const {
        auto out = ctx.out();
        ((out = format_element<Is>(t, out)), ...);
        return out;
    }
};
