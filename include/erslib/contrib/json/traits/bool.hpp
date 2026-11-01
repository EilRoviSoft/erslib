#pragma once

// std
#include <concepts>

// ers
#include <erslib/contrib/json/value.hpp>


template<std::same_as<bool> T>
struct utl::impl::json_value<T> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, T> read(const Node& json) {
        if (json.is_bool())
            return json.as_bool();

        return wrong_type<T, Policy>("bool", json);
    }

    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, T what) {
        out = what;
        return Policy::done();
    }
};
