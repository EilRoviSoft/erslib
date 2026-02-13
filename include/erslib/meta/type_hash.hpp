#pragma once

// ers
#include <erslib/hashing/rapid.hpp>
#include <erslib/meta/type_name.hpp>


namespace ers::meta {
    template<typename T, typename Hasher = RapidHash<std::string_view>>
    struct type_hash {
        static constexpr auto value = Hasher {}(type_name<T>::value);
    };

    template<typename T, typename Hasher = RapidHash<std::string_view>>
    constexpr auto type_hash_v = type_hash<T, Hasher>::value;
}
