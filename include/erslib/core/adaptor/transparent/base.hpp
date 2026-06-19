#pragma once

// std
#include <concepts>
#include <type_traits>

// ers
#include <erslib/core/trait/member.hpp>


// Helpers

namespace ers::internal {
    template<auto Member>
    concept MemberOrNull = std::same_as<decltype(Member), std::nullptr_t> || std::is_member_pointer_v<decltype(Member)>;

    template<auto Member>
    static constexpr bool has_member_v = !std::same_as<decltype(Member), std::nullptr_t>;

    template<auto Member, typename T>
    using projected_t = std::remove_cvref_t<
        std::invoke_result_t<decltype(Member), T>
    >;


    template<typename T>
    constexpr decltype(auto) enum_arg(T&& v) noexcept {
        if constexpr (std::is_enum_v<std::remove_cvref_t<T>>) {
            return std::to_underlying(v);
        } else {
            return std::forward<T>(v);
        }
    }


    template<auto Member, typename T>
    consteval bool project_is_nothrow() {
        if constexpr (has_member_v<Member> && std::invocable<decltype(Member), T&&>) {
            return noexcept(std::invoke(Member, std::declval<T&&>()));
        } else {
            return true;
        }
    }

    template<auto Member, typename T>
    constexpr decltype(auto) project(T&& u) noexcept(project_is_nothrow<Member, T>()) {
        if constexpr (has_member_v<Member> && std::invocable<decltype(Member), T&&>) {
            return std::invoke(Member, std::forward<T>(u));
        } else {
            return std::forward<T>(u);
        }
    }


    template<typename T, auto Member>
    consteval bool owner_matches() {
        if constexpr (has_member_v<Member>) {
            return std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<member_class_t<Member>>>;
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


    template<size_t Arity, typename T, typename Fn, auto Member>
        requires MemberOrNull<Member>
    struct op_base {
        using is_transparent = void;
        using type = std::remove_cvref_t<T>;


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

namespace ers::internal {
    template<template<typename> typename Hash, auto Member, typename... Ts>
    struct hash_base;


    template<template<typename> typename Hash, auto Member, typename T, typename... Ts>
        requires (Member != nullptr)
    struct hash_base<Hash, Member, T, Ts...> : Hash<std::remove_cvref_t<Ts>>... {
        using is_transparent = void;

        using primary_type = std::remove_cvref_t<T>;
        using secondary_types = std::tuple<std::remove_cvref_t<Ts>...>;
        using projected_type = std::remove_cvref_t<std::invoke_result_t<decltype(Member), const primary_type&>>;


        static_assert(owner_matches<primary_type, Member>(), "T must match the class owning Member");


        using Hash<std::remove_cvref_t<Ts>>::operator()...;

        constexpr size_t operator()(
            const primary_type& v, size_t seed = 0
        ) const noexcept(noexcept(invoke_projected<Hash<projected_type>, Member>(v, seed))
        ) requires (ProjectedInvocable<Hash<projected_type>, Member, const primary_type&, size_t>) {
            return invoke_projected<Hash<projected_type>, Member>(v, seed);
        }
    };

    template<template<typename> typename Hash, typename... Ts>
    struct hash_base<Hash, nullptr, Ts...> : Hash<std::remove_cvref_t<Ts>>... {
        using is_transparent = void;
        using types = std::tuple<std::remove_cvref_t<Ts>...>;


        using Hash<std::remove_cvref_t<Ts>>::operator()...;
    };
}


// Generic implementations

namespace ers::adaptor {
    template<typename T, typename Fn>
    using unary_op = internal::op_base<1, T, Fn, nullptr>;

    template<auto Member, typename Fn>
    using member_unary_op = internal::op_base<1, std::remove_cvref_t<member_class_t<Member>>, Fn, Member>;


    template<typename T, typename Fn>
    using binary_op = internal::op_base<2, T, Fn, nullptr>;

    template<auto Member, typename Fn>
    using member_binary_op = internal::op_base<2, std::remove_cvref_t<member_class_t<Member>>, Fn, Member>;
}


// Specialized implementations

namespace ers {
    template<template<typename> typename Hash, typename... Ts>
    using hash_adaptor = internal::hash_base<Hash, nullptr, Ts...>;

    template<template<typename> typename Hash, auto Member, typename... Ts>
    using member_hash_adaptor = internal::hash_base<Hash, Member, std::remove_cvref_t<member_class_t<Member>>, Ts...>;
}

namespace ers {
    template<typename T>
    using equal_adaptor = adaptor::binary_op<T, std::equal_to<>>;

    template<auto Member>
    using member_equal_adaptor = adaptor::member_binary_op<Member, std::equal_to<>>;
}
