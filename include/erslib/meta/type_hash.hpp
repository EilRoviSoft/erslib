#pragma once

// ers
#include <erslib/hashing/rapid.hpp>
#include <erslib/meta/type_name.hpp>

namespace ers::meta {
    template<typename T, typename Hasher = RapidMicroHash<std::string_view>>
    constexpr size_t type_hash() {
        constexpr auto name = type_name<T>();
        return Hasher {}(name);
    }
}
