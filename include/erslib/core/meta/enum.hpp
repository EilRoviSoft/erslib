#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/core/meta/enum requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif


// std
#include <meta>


// Enumerator lookup

namespace ers::impl::meta {
    template<typename E, E V>
    consteval std::meta::info enumerator_of() {
        std::meta::info result {};

        template for (constexpr auto e : std::define_static_array(std::meta::enumerators_of(^^E))) {
            if constexpr (std::meta::extract<E>(e) == V)
                result = e;
        }

        return result;
    }
}


// Annotation queries

namespace ers::impl::meta {
    template<typename Attr, typename E, E V>
    consteval bool has_attribute() {
        return !std::meta::annotations_of_with_type(enumerator_of<E, V>(), ^^Attr).empty();
    }

    template<template<auto> typename Attr, typename E, E V>
    consteval bool has_template_attribute() {
        template for (constexpr auto attr : std::define_static_array(std::meta::annotations_of(enumerator_of<E, V>()))) {
            constexpr auto type = std::meta::remove_const(std::meta::type_of(attr));

            if constexpr (std::meta::has_template_arguments(type)) {
                if constexpr (std::meta::template_of(type) == ^^Attr)
                    return true;
            }
        }

        return false;
    }

    template<template<auto> typename Attr, typename E, E V>
    consteval auto get_template_attribute_value() {
        template for (constexpr auto attr : std::define_static_array(std::meta::annotations_of(enumerator_of<E, V>()))) {
            constexpr auto type = std::meta::remove_const(std::meta::type_of(attr));

            if constexpr (std::meta::has_template_arguments(type)) {
                if constexpr (std::meta::template_of(type) == ^^Attr) {
                    using T = [:type:];
                    constexpr T obj = std::meta::extract<T>(attr);
                    return obj.value;
                }
            }
        }
    }
}
