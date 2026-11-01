#pragma once

// std
#include <cmath>
#include <concepts>
#include <utility>

// ers
#include <erslib/contrib/json/value.hpp>


template<typename T>
    requires (std::integral<T> && !std::same_as<T, bool>)
struct utl::impl::json_value<T> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, T> read(const Node& json) {
        if (!json.is_integral())
            return wrong_type<T, Policy>("integer", json);

        auto value = json.as_integral();

        if (!std::in_range<T>(value)) {
            return Policy::template fail<T>([&] {
                return std::format("{} is out of range", value);
            });
        }

        return static_cast<T>(value);
    }

    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, T what) {
        if (!std::in_range<typename Node::integral_type>(what)) {
            return Policy::template fail<void>([&] {
                return std::format("{} doesn't fit into a JSON integer", what);
            });
        }

        out = what;
        return Policy::done();
    }
};

template<std::floating_point T>
struct utl::impl::json_value<T> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, T> read(const Node& json) {
        if (json.is_floating())
            return static_cast<T>(json.as_floating());

        if (json.is_integral())
            return static_cast<T>(json.as_integral());

        return wrong_type<T, Policy>("number", json);
    }

    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, T what) {
        if (!std::isfinite(what)) {
            return Policy::template fail<void>([&] {
                return std::format("{} can't be written as a JSON number", what);
            });
        }

        out = what;
        return Policy::done();
    }
};
