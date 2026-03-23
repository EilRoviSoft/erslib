#pragma once

// std
#include <concepts>
#include <functional>
#include <type_traits>

// ers
#include <erslib/adaptor/transparent/base.hpp>
#include <erslib/trait/member.hpp>
#include <erslib/trait/transparent.hpp>


// Generic implementations

namespace ers::adaptor {
    template<auto Member, typename Fn>
    struct unary_op<std::remove_cv_t<member_class_t<Member>>, Fn, Member> {
        using is_transparent = void;
        using object_type = std::remove_cv_t<member_class_t<Member>>;


        auto operator()(
            const object_type& v
        ) const noexcept(noexcept(Fn {}(v.*Member))
        ) {
            return Fn {}(v.*Member);
        }

        template<typename T>
        auto operator()(
            const T& v
        ) const noexcept(noexcept(Fn {}(v))
        ) requires (!std::same_as<std::remove_cvref_t<T>, object_type>) {
            return Fn {}(v);
        }
    };

    template<auto Member, typename Fn>
    struct binary_op<std::remove_cv_t<member_class_t<Member>>, Fn, Member> {
        using is_transparent = void;
        using object_type = std::remove_cv_t<member_class_t<Member>>;


        auto operator()(
            const object_type& l,
            const object_type& r
        ) const noexcept(noexcept(Fn {}(l.*Member, r.*Member))
        ) {
            return Fn {}(l.*Member, r.*Member);
        }

        template<typename T>
        auto operator()(
            const object_type& l,
            const T& r
        ) const noexcept(noexcept(Fn {}(l, r))
        ) requires (!std::same_as<std::remove_cvref_t<T>, object_type>) {
            return Fn {}(l, r);
        }

        template<typename T>
        auto operator()(
            const T& l,
            const object_type& r
        ) const noexcept(noexcept(Fn {}(l, r))
        ) requires (!std::same_as<std::remove_cvref_t<T>, object_type>) {
            return Fn {}(l, r);
        }
    };
}


namespace ers::adaptor {
    template<auto Member, typename Fn>
    using projected_unary_op = unary_op<std::remove_cv_t<member_class_t<Member>>, Fn, Member>;

    template<auto Member, typename Fn>
    using projected_binary_op = binary_op<std::remove_cv_t<member_class_t<Member>>, Fn, Member>;
}


// Specialized implementations

namespace ers::adaptor {
    template<auto Member, typename Policy>
    using projected_hash = projected_unary_op<Member, transparent_hash_for_t<member_type_t<Member>, Policy>>;

    template<auto Member>
    using projected_equal = projected_binary_op<Member, std::equal_to<>>;
}
