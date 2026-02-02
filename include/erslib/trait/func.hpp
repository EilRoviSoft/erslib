#pragma once

// std
#include <tuple>

// ers
#include <erslib/concept/callable.hpp>

namespace ers::trait {
    template<typename T>
    struct function;

    template<typename TRet, typename... TArgs>
    struct function<TRet(*)(TArgs...)> {
        using return_type = TRet;
        using args_tuple = std::tuple<TArgs...>;
        static constexpr std::size_t arity = sizeof...(TArgs);

        template<size_t TIndex>
        using arg_type = std::tuple_element_t<TIndex, args_tuple>;
    };

    template<typename TRet, typename... TArgs>
    struct function<TRet(TArgs...)> : function<TRet(*)(TArgs...)> {};

    template<typename TClass, typename TRet, typename... TArgs>
    struct function<TRet(TClass::*)(TArgs...)> : function<TRet(*)(TArgs...)> {};

    template<typename TClass, typename TRet, typename... TArgs>
    struct function<TRet(TClass::*)(TArgs...) const> : function<TRet(*)(TArgs...)> {};

    template<Callable T>
    struct function<T> : function<decltype(&T::operator())> {};
}
