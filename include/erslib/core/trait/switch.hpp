#pragma once

// std
#include <type_traits>


namespace ers::impl {
    template<auto Value, typename T>
    struct case_t {};

    template<typename T>
    struct default_t {};


    template<auto Value, typename... Cases>
    struct type_switch;

    template<auto Value>
    struct type_switch<Value> {
        static_assert(false, "type_switch: no case matches the value and there is no default_t");
    };

    template<auto Value, auto CaseValue, typename T, typename... Rest>
    struct type_switch<Value, case_t<CaseValue, T>, Rest...> :
        std::conditional_t<Value == CaseValue, std::type_identity<T>, type_switch<Value, Rest...>> {
    };

    template<auto Value, typename T, typename... Rest>
    struct type_switch<Value, default_t<T>, Rest...> : std::type_identity<T> {
        static_assert(sizeof...(Rest) == 0, "type_switch: default_t must be the last one");
    };

    template<auto Value, typename... Cases>
    using type_switch_t = typename type_switch<Value, Cases...>::type;
}


// Exports ---------------------------------------------------------------------

namespace ers {
    using impl::case_t;
    using impl::default_t;

    using impl::type_switch;
    using impl::type_switch_t;
}
