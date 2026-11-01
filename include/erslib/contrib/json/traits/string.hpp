#pragma once

// std
#include <concepts>
#include <string>
#include <string_view>

// ers
#include <erslib/contrib/json/value.hpp>


template<std::same_as<std::string> T>
struct utl::impl::json_value<T> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, T> read(const Node& json) {
        if (json.is_string())
            return utf8_of(json.as_string());

        return wrong_type<T, Policy>("string", json);
    }

    template<JsonNode Node>
    static void write(Node& out, const T& what) {
        out = std::string_view(what);
    }
};
