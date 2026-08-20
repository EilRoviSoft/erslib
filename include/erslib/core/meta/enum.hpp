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
//
// `Target` is the reflection of whatever is being inspected for annotations -
// an enumerator, found via enumerator_of<E, V>().

namespace ers::impl::meta {
    template<std::meta::info Target, typename Attr>
    consteval bool has_attribute() {
        return !std::meta::annotations_of_with_type(Target, ^^Attr).empty();
    }


    template<std::meta::info Target, template<auto...> typename Attr>
    consteval bool has_template_attribute() {
        template for (constexpr auto attr : std::define_static_array(std::meta::annotations_of(Target))) {
            constexpr auto type = std::meta::remove_const(std::meta::type_of(attr));

            if constexpr (std::meta::has_template_arguments(type)) {
                if constexpr (std::meta::template_of(type) == ^^Attr)
                    return true;
            }
        }

        return false;
    }

    template<std::meta::info Target, template<auto...> typename Attr, size_t I = 0>
    consteval auto get_template_attribute_arg() {
        template for (constexpr auto attr : std::define_static_array(std::meta::annotations_of(Target))) {
            constexpr auto type = std::meta::remove_const(std::meta::type_of(attr));

            if constexpr (std::meta::has_template_arguments(type)) {
                if constexpr (std::meta::template_of(type) == ^^Attr) {
                    constexpr auto arg = std::meta::template_arguments_of(type)[I];
                    using ArgT = [:std::meta::type_of(arg):];
                    return std::meta::extract<ArgT>(arg);
                }
            }
        }
    }
}
