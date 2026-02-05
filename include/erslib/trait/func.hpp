#pragma once

// std
#include <tuple>

// ers
#include <erslib/concept/callable.hpp>

namespace ers::trait {
    template<typename T>
    struct function;

    template<typename R, typename... Args>
    struct function<R(*)(Args...)> {
        using return_type = R;
        using args_tuple = std::tuple<Args...>;
        static constexpr std::size_t arity = sizeof...(Args);

        template<size_t TIndex>
        using arg_type = std::tuple_element_t<TIndex, args_tuple>;
    };

    template<typename R, typename... Args>
    struct function<R(Args...)> : function<R(*)(Args...)> {};

    template<typename T, typename R, typename... Args>
    struct function<R(T::*)(Args...)> : function<R(*)(Args...)> {};

    template<typename T, typename R, typename... Args>
    struct function<R(T::*)(Args...) const> : function<R(*)(Args...)> {};

    template<Callable T>
    struct function<T> : function<decltype(&T::operator())> {};
}
