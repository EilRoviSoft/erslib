#pragma once

// std
#include <concepts>
#include <string>
#include <string_view>

// ers
#include <erslib/contrib/json/value.hpp>


namespace utl::impl {
    template<typename T>
    struct json_text;

    template<typename T>
    concept JsonText = requires(std::string_view text, const T& what) {
        { json_text<T>::try_parse(text) } -> std::same_as<ers::Result<T>>;
        { json_text<T>::maybe_parse(text) } -> std::same_as<ers::maybe<T>>;
        { json_text<T>::to_text(what) } -> std::convertible_to<std::string>;
    };
}


template<utl::impl::JsonText T>
struct utl::impl::json_value<T> {
    using codec = json_text<T>;


    template<typename Policy, JsonNode Node>
    static read_t<Policy, T> read(const Node& json) {
        if (!json.is_string())
            return wrong_type<T, Policy>("string", json);

        return Policy::template parse<codec>(utf8_of(json.as_string()));
    }

    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, const T& what) {
        out = codec::to_text(what);
        return Policy::done();
    }
};
