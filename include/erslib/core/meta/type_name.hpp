#pragma once

// ers
#include <erslib/core/meta/impl/type_name_array.hpp>


namespace ers::meta {
    template<typename T>
    struct type_name {
        static constexpr auto value = internal::funcsig<T>();
    };

    template<typename T>
    constexpr std::string_view type_name_v = type_name<T>::value;
}
