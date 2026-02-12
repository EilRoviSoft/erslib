#pragma once

// ers
#include <erslib/hashing/rapid.hpp>
#include <erslib/meta/type_name.hpp>

namespace ers::meta {
    template<typename T, typename Hasher = RapidMicroHash<std::string_view>>
    struct type_hash {
        constexpr size_t value = Hasher {}(type_name<T>::value);
    };

    template<typename T, typename Hasher = RapidMicroHash<std::string_view>>
    constexpr size_t type_hash_v = type_hash<T, Hasher>::value;
}
