#pragma once

// std
#include <utility>


namespace ers {
    template<typename T>
    struct member_ptr_traits {};

    template<typename Class, typename Member>
    struct member_ptr_traits<Class Member::*> {
        using class_type = Class;
        using member_type = Member;
    };


    template<auto Member>
    using member_class_t = typename member_ptr_traits<decltype(Member)>::class_type;

    template<auto Member>
    using member_type_t = std::remove_cvref_t<typename member_ptr_traits<decltype(Member)>::member_type>;
}


namespace ers {
    template<auto Member>
    struct projected {
        template<typename T>
        static constexpr decltype(auto) get(T&& v) {
            if constexpr (requires { std::forward<T>(v).*Member; })
                return std::forward<T>(v).*Member;
            else
                return std::forward<T>(v);
        }
    };
}
