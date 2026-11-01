#pragma once

// ers
#include <erslib/contrib/json/value.hpp>
#include <erslib/core/type/maybe.hpp>


template<typename T>
struct utl::impl::json_value<ers::maybe<T>> {
    template<JsonNode Node>
    static ers::Result<ers::maybe<T>> read(const Node& json) {
        if (json.is_null())
            return ers::maybe<T> {};

        auto value = json_value<T>::read(json);

        if (!value)
            return value.error();

        return ers::maybe<T> { std::move(*value) };
    }

    template<JsonNode Node>
    static void write(Node& out, const ers::maybe<T>& what) {
        if (what)
            json_value<T>::write(out, *what);
        else
            out = nullptr;
    }
};
