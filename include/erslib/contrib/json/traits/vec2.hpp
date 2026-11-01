#pragma once

// std
#include <utility>

// ers
#include <erslib/contrib/json/value.hpp>
#include <erslib/core/type/general.hpp>


template<typename T>
struct utl::impl::json_value<vec2<T>> {
    template<JsonNode Node>
    static ers::Result<vec2<T>> read(const Node& json) {
        if (!json.is_object())
            return expected_error("object", json);

        vec2<T> out;

        for (auto [name, member] : { std::pair { "x", &vec2<T>::x }, std::pair { "y", &vec2<T>::y } }) {
            auto node = json.try_at(name);

            if (!node)
                return ers::make_error("no field '{}'", name);

            auto value = json_value<T>::read(*node)
                .context("'{}'", name);

            if (!value)
                return value.error();

            out.*member = *value;
        }

        return out;
    }

    template<JsonNode Node>
    static void write(Node& out, const vec2<T>& what) {
        out = typename Node::object_type {};

        json_value<T>::write(out["x"], what.x);
        json_value<T>::write(out["y"], what.y);
    }
};
