#pragma once

// std
#include <utility>

// ers
#include <erslib/contrib/json/value.hpp>
#include <erslib/core/type/general.hpp>


template<typename T>
struct utl::impl::json_value<vec2<T>> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, vec2<T>> read(const Node& json) {
        if (!json.is_object())
            return wrong_type<vec2<T>, Policy>("object", json);

        vec2<T> out;

        for (auto [name, member] : { std::pair { "x", &vec2<T>::x }, std::pair { "y", &vec2<T>::y } }) {
            auto node = json.try_at(name);

            if (!node) {
                return Policy::template fail<vec2<T>>([&] {
                    return std::format("no field '{}'", name);
                });
            }

            auto value = json_value<T>::template read<Policy>(*node);

            if (!value)
                return Policy::template propagate<vec2<T>>(std::move(value), "'{}'", name);

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
