#pragma once

// ers
#include <erslib/hashing/rapid.hpp>
#include <erslib/meta/type_name.hpp>

namespace ers::meta {
    template<typename T, typename Hasher = RapidMicroHash<std::string_view>>
    struct type_hash {
        constexpr auto value = Hasher {}(type_name<T>::value);
    };
}
