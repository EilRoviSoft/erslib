#pragma once

// ers
#include <erslib/contrib/json/value.hpp>
#include <erslib/core/type/maybe.hpp>


template<typename T>
struct utl::impl::json_value<ers::maybe<T>> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, ers::maybe<T>> read(const Node& json) {
        using result_t = read_t<Policy, ers::maybe<T>>;

        if (json.is_null())
            return result_t(ers::maybe<T> {});

        auto value = json_value<T>::template read<Policy>(json);

        if (!value)
            return Policy::template propagate<ers::maybe<T>>(std::move(value));

        return result_t(ers::maybe<T> { std::move(*value) });
    }

    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, const ers::maybe<T>& what) {
        if (!what) {
            out = nullptr;
            return Policy::done();
        }

        return json_value<T>::template write<Policy>(out, *what);
    }
};
