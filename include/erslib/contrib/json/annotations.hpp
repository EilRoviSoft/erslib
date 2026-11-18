#pragma once

// std
#include <array>
#include <meta>
#include <string_view>
#include <type_traits>


// Object

namespace utl::impl {
    struct json_object_t {};

    inline constexpr json_object_t json_object {};


    template<typename T>
    inline constexpr bool is_json_object_v = false;

    template<typename T>
    concept JsonObjectType = std::is_class_v<T>
        && (is_json_object_v<T> || !std::meta::annotations_of_with_type(^^T, ^^json_object_t).empty());


    template<typename T>
    inline constexpr auto members_of_v = std::define_static_array(
        std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));
}


// Skip

namespace utl::impl {
    struct json_skip_t {};

    inline constexpr json_skip_t json_skip {};


    template<std::meta::info M>
    consteval bool is_json_skipped() {
        return !std::meta::annotations_of_with_type(M, ^^json_skip_t).empty();
    }
}


// Key

namespace utl::impl {
    struct json_key_t {
        const char* name;

        consteval json_key_t(std::string_view value) :
            name(std::define_static_string(value)) {
        }
    };


    template<std::meta::info M>
    consteval std::string_view json_key_of() {
        auto keys = std::meta::annotations_of_with_type(M, ^^json_key_t);

        if (!keys.empty())
            return std::meta::extract<json_key_t>(keys[0]).name;

        return std::meta::identifier_of(M);
    }
}


// Tag

namespace utl::impl {
    struct json_tag_t {
        const char* name;

        consteval json_tag_t(std::string_view value) :
            name(std::define_static_string(value)) {
        }
    };


    template<typename T>
    consteval bool has_json_tag() {
        return !std::meta::annotations_of_with_type(^^T, ^^json_tag_t).empty();
    }

    template<typename T>
    consteval std::string_view json_tag_of() {
        return std::meta::extract<json_tag_t>(std::meta::annotations_of_with_type(^^T, ^^json_tag_t)[0]).name;
    }

    template<typename T>
    consteval bool has_json_key(std::string_view key) {
        template for (constexpr auto m : members_of_v<T>) {
            if constexpr (!is_json_skipped<m>()) {
                if (json_key_of<m>() == key)
                    return true;
            }
        }

        return false;
    }

    template<typename... Ts>
    consteval bool json_tags_distinct() {
        constexpr std::array<std::string_view, sizeof...(Ts)> tags { json_tag_of<Ts>()... };

        for (size_t i = 0; i < tags.size(); ++i)
            for (size_t j = i + 1; j < tags.size(); ++j)
                if (tags[i] == tags[j])
                    return false;

        return true;
    }

    template<typename... Ts>
    consteval bool json_types_distinct() {
        constexpr std::array<std::meta::info, sizeof...(Ts)> types { ^^Ts... };

        for (size_t i = 0; i < types.size(); ++i)
            for (size_t j = i + 1; j < types.size(); ++j)
                if (types[i] == types[j])
                    return false;

        return true;
    }
}
