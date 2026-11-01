#pragma once

// std
#include <type_traits>

// ers
#include <erslib/contrib/json/value.hpp>
#include <erslib/core/enum.hpp>


template<typename E>
    requires std::is_enum_v<E>
struct utl::impl::json_value<E> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, E> read(const Node& json) {
        if (!json.is_string())
            return wrong_type<E, Policy>("string", json);

        auto text = utf8_of(json.as_string());

        if (auto parsed = ers::enum_utils::from_string<E>(text))
            return *parsed;

        return Policy::template fail<E>([&] {
            return std::format("unknown value '{}'", text);
        });
    }

    template<JsonNode Node>
    static void write(Node& out, E what) {
        out = ers::enum_utils::to_string(what);
    }
};
