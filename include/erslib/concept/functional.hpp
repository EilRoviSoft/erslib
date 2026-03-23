#pragma once

// std
#include <type_traits>

// ers
#include <erslib/concept/functional.hpp>

#include "erslib/hashing/base.hpp"


// Generic implementations

namespace ers {
    template<typename Fn, typename T, typename E>
    concept UnaryWith = requires(T&& v) {
        { Fn {}(std::forward<T>(v)) } -> std::convertible_to<E>;
    };

    template<typename Fn, typename L, typename R, typename E>
    concept BinaryWith = requires(L&& l, R&& r) {
        { Fn {}(std::forward<L>(l), std::forward<R>(r)) } -> std::convertible_to<E>;
    };

    template<typename Fn, typename... Args, typename E>
    concept ExecutableWith = requires(Args&&... args) {
        { Fn {}(std::forward<Args>(args)...) } -> std::convertible_to<E>;
    };
}


// Specialized implementations

namespace ers {
    template<typename Policy, typename T>
    concept HashableWith = ExecutableWith<THashBase<T, Policy>, T, size_t, size_t>;


    template<typename Equal, typename L, typename R>
    concept EqualWith = BinaryWith<Equal, L, R, bool>;
}

