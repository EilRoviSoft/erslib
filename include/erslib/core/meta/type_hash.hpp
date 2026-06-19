#pragma once

// ers
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/meta/impl/type_name_array.hpp>


namespace ers::meta {
    template<typename T, typename Policy = rapid_hash_policy>
    struct type_hash {
        using name_type = std::remove_cvref_t<decltype(internal::type_name_array<T>())>;

        static constexpr auto value = THashBase<name_type, Policy> {}(internal::type_name_array<T>());
    };

    template<typename T, typename Policy = rapid_hash_policy>
    constexpr auto type_hash_v = type_hash<T, Policy>::value;
}
