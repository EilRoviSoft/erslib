#pragma once

// std
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <type_traits>

// crow
#include <crow/app.h>
#include <crow/middleware_context.h>


namespace ers::web::impl {
    template<typename AllContext, typename Middleware>
    struct context_has : std::false_type {};

    template<typename... Middlewares, typename Middleware>
    struct context_has<crow::detail::context<Middlewares...>, Middleware> :
        std::bool_constant<(std::same_as<Middlewares, Middleware> || ...)> {
    };


    template<typename AllContext, typename... Middlewares>
    concept ContextWith = (context_has<std::remove_cvref_t<AllContext>, Middlewares>::value && ...);


    template<typename T, typename... Ts>
    constexpr size_t index_of = [] {
        constexpr bool same[] { std::same_as<T, Ts>..., false };
        return static_cast<size_t>(std::ranges::find(same, true) - std::begin(same));
    }();

    template<typename App, typename First, typename Second>
    constexpr bool runs_before = false;

    template<typename... Middlewares, typename First, typename Second>
    constexpr bool runs_before<crow::Crow<Middlewares...>, First, Second> =
        index_of<First, Middlewares...> < index_of<Second, Middlewares...>
        && index_of<Second, Middlewares...> < sizeof...(Middlewares);
}


namespace ers::web {
    using impl::ContextWith;
    using impl::runs_before;
}
