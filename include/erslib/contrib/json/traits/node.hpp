#pragma once

// ers
#include <erslib/contrib/json/value.hpp>


template<utl::impl::JsonNode T>
struct utl::impl::json_value<T> {
    template<JsonNode Node>
    static ers::Result<T> read(const Node& json) {
        return T(json);
    }

    template<JsonNode Node>
    static void write(Node& out, const T& what) {
        out = Node(what);
    }
};
