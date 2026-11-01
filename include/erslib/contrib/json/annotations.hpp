#pragma once

// std
#include <meta>
#include <string_view>
#include <type_traits>


namespace utl::impl {
    struct json_object_t {};

    inline constexpr json_object_t json_object {};

    struct json_key_t {
        const char* name;

        consteval json_key_t(const char* value) :
            name(value) {
        }
    };


    template<typename T>
    inline constexpr bool is_json_object_v = false;

    template<typename T>
    concept JsonObjectType = std::is_class_v<T>
        && (is_json_object_v<T> || !std::meta::annotations_of_with_type(^^T, ^^json_object_t).empty());

    template<std::meta::info M>
    consteval std::string_view json_key_of() {
        auto keys = std::meta::annotations_of_with_type(M, ^^json_key_t);

        if (!keys.empty())
            return std::meta::extract<json_key_t>(keys[0]).name;

        return std::meta::identifier_of(M);
    }

    template<typename T>
    inline constexpr auto members_of_v = std::define_static_array(
        std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
}
