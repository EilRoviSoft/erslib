#pragma once

// std
#include <concepts>
#include <type_traits>

// ers
#include <erslib/hashing/base.hpp>
#include <erslib/trait/member.hpp>


// Helpers

namespace ers::internal {
    template<typename T>
    using raw_t = std::remove_cvref_t<T>;


    template<auto Member>
    concept MemberOrNull = std::same_as<decltype(Member), std::nullptr_t> || std::is_member_object_pointer_v<decltype(Member)>;

    template<auto Member>
    static constexpr bool has_member_v = !std::same_as<decltype(Member), std::nullptr_t>;


    template<typename T>
    constexpr decltype(auto) enum_arg(T&& v) noexcept {
        if constexpr (std::is_enum_v<raw_t<T>>) {
            return std::to_underlying(v);
        } else {
            return std::forward<T>(v);
        }
    }


    template<auto Member, typename U>
    consteval bool project_is_nothrow() {
        if constexpr (has_member_v<Member> && std::invocable<decltype(Member), U&&>) {
            return noexcept(std::invoke(Member, std::declval<U&&>()));
        } else {
            return true;
        }
    }

    template<auto Member, typename U>
    constexpr decltype(auto) project(U&& u) noexcept(project_is_nothrow<Member, U>()) {
        if constexpr (has_member_v<Member> && std::invocable<decltype(Member), U&&>) {
            return std::invoke(Member, std::forward<U>(u));
        } else {
            return std::forward<U>(u);
        }
    }


    template<typename T, auto Member>
    consteval bool owner_matches() {
        if constexpr (has_member_v<Member>) {
            return std::same_as<std::remove_cv_t<T>, std::remove_cv_t<member_class_t<Member>>>;
        } else {
            return true;
        }
    }


    template<typename Fn, auto Member, typename... Args>
    concept DirectlyProjectedInvocable = requires(Args&&... args) {
        std::invoke(Fn {}, project<Member>(std::forward<Args>(args))...);
    };

    template<typename Fn, auto Member, typename... Args>
    concept EnumProjectedInvocable = requires(Args&&... args) {
        std::invoke(Fn {}, enum_arg(project<Member>(std::forward<Args>(args)))...);
    };

    template<typename Fn, auto Member, typename... Args>
    concept ProjectedInvocable = DirectlyProjectedInvocable<Fn, Member, Args...> || EnumProjectedInvocable<Fn, Member, Args...>;


    template<typename Fn, auto Member, typename... Args>
    consteval bool invoke_is_nothrow() {
        if constexpr (DirectlyProjectedInvocable<Fn, Member, Args...>) {
            return noexcept(std::invoke(Fn {}, project<Member>(std::declval<Args>())...));
        } else {
            return noexcept(std::invoke(Fn {}, enum_arg(project<Member>(std::declval<Args>()))...));
        }
    }

    template<typename Fn, auto Member, typename... Args>
    constexpr decltype(auto) invoke_projected(
        Args&&... args
    ) noexcept(invoke_is_nothrow<Fn, Member, Args...>()
    ) requires ProjectedInvocable<Fn, Member, Args...> {
        if constexpr (DirectlyProjectedInvocable<Fn, Member, Args...>) {
            return std::invoke(Fn {}, project<Member>(std::forward<Args>(args))...);
        } else {
            return std::invoke(Fn {}, enum_arg(project<Member>(std::forward<Args>(args)))...);
        }
    }


    template<std::size_t Arity, typename T, typename Fn, auto Member>
        requires MemberOrNull<Member>
    struct op_base {
        using is_transparent = void;
        using type = std::remove_cv_t<T>;
        using object_type = type;

        static_assert(owner_matches<T, Member>(), "T must match the class owning Member");

        template<typename... Args>
        constexpr decltype(auto) operator()(
            Args&&... args
        ) const noexcept(noexcept(invoke_projected<Fn, Member>(std::forward<Args>(args)...))
        ) requires (sizeof...(Args) == Arity && ProjectedInvocable<Fn, Member, Args...>) {
            return invoke_projected<Fn, Member>(std::forward<Args>(args)...);
        }
    };
}


// Generic implementations

namespace ers::adaptor {
    template<typename T, typename Fn, auto Member = nullptr>
    using unary_op = internal::op_base<1, T, Fn, Member>;

    template<auto Member, typename Fn>
    using member_unary_op = unary_op<std::remove_cv_t<member_class_t<Member>>, Fn, Member>;


    template<typename T, typename Fn, auto Member = nullptr>
    using binary_op = internal::op_base<2, T, Fn, Member>;

    template<auto Member, typename Fn>
    using member_binary_op = binary_op<std::remove_cv_t<member_class_t<Member>>, Fn, Member>;
}


// Specialized implementations

namespace ers {
    template<typename T, typename Policy>
    using hash_adaptor = adaptor::unary_op<T, THashBase<T, Policy>>;

    template<auto Member, typename Policy>
    using member_hash_adaptor = adaptor::member_unary_op<Member, THashBase<std::remove_cv_t<member_class_t<Member>>, Policy>>;


    template<typename T>
    using equal_adaptor = adaptor::binary_op<T, std::equal_to<>>;

    template<auto Member>
    using member_equal_adaptor = adaptor::member_binary_op<Member, std::equal_to<>>;
}
