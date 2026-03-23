#pragma once

// std
#include <concepts>
#include <type_traits>


// Forward declaration

namespace ers::adaptor {
    template<typename T, typename Fn, auto Member = nullptr>
    struct unary_op;

    template<typename T, typename Fn, auto Member = nullptr>
    struct binary_op;
}



// Generic implementations

namespace ers::adaptor {
    template<typename T, typename Fn>
    struct unary_op<T, Fn, nullptr> {
        using is_transparent = void;
        using type = T;


        auto operator()(const type& v) const noexcept(noexcept(Fn {}(v))) {
            return Fn {}(v);
        }

        template<typename U>
        auto operator()(
            const U& v
        ) const noexcept(noexcept(Fn {}(v))
        ) requires (!std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>) {
            return Fn {}(v);
        }
    };

    template<typename T, typename Fn>
    struct binary_op<T, Fn, nullptr> {
        using is_transparent = void;
        using type = T;


        auto operator()(const T& l, const T& r) const noexcept(noexcept(Fn {}(l, r))) {
            return Fn {}(l, r);
        }

        template<typename U>
        auto operator()(
            const T& l, const U& r
        ) const noexcept(noexcept(Fn {}(l, r))
        ) requires (!std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>) {
            return Fn {}(l, r);
        }

        template<typename U>
        auto operator()(
            const U& l, const T& r
        ) const noexcept(noexcept(Fn {}(l, r))
        ) requires (!std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>) {
            return Fn {}(l, r);
        }
    };
}


// Specialized implementations

namespace ers::adaptor {
    template<typename T>
    using equal = binary_op<T, std::equal_to<>>;
}
