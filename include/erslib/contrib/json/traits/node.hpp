#pragma once

// ers
#include <erslib/contrib/json/value.hpp>


template<utl::impl::JsonNode T>
struct utl::impl::json_value<T> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, T> read(const Node& json) {
        return T(json);
    }

    template<JsonNode Node>
    static void write(Node& out, const T& what) {
        out = Node(what);
    }
};
