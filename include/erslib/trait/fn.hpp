#pragma once

// std
#include <tuple>

// ers
#include <erslib/concept/callable.hpp>


namespace ers {
    template<typename T>
    struct fn_traits;

    template<typename R, typename... Args>
    struct fn_traits<R(*)(Args...)> {
        using return_type = R;
        using args_tuple = std::tuple<Args...>;
        static constexpr std::size_t arity = sizeof...(Args);

        template<size_t TIndex>
        using arg_type = std::tuple_element_t<TIndex, args_tuple>;
    };

    template<typename R, typename... Args>
    struct fn_traits<R(Args...)> : fn_traits<R(*)(Args...)> {};

    template<typename T, typename R, typename... Args>
    struct fn_traits<R(T::*)(Args...)> : fn_traits<R(*)(Args...)> {};

    template<typename T, typename R, typename... Args>
    struct fn_traits<R(T::*)(Args...) const> : fn_traits<R(*)(Args...)> {};

    template<typename T>
        requires Callable<T>
    struct fn_traits<T> : fn_traits<decltype(&T::operator())> {};
}
