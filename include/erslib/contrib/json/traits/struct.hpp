#pragma once

// ers
#include <erslib/contrib/json/annotations.hpp>
#include <erslib/contrib/json/value.hpp>


template<utl::impl::JsonObjectType T>
struct utl::impl::json_value<T> {
    template<JsonNode Node>
    static ers::Result<T> read(const Node& json) {
        if (!json.is_object())
            return expected_error("object", json);

        T out {};

        template for (constexpr auto m : members_of_v<T>) {
            constexpr auto key = json_key_of<m>();
            using F = [:std::meta::type_of(m):];

            if (auto node = json.try_at(key)) {
                auto value = json_value<F>::read(*node)
                    .context("'{}'", key);

                if (!value)
                    return value.error();

                out.[:m:] = std::move(*value);
            }
        }

        return out;
    }

    template<JsonNode Node>
    static void write(Node& out, const T& what) {
        out = typename Node::object_type {};

        template for (constexpr auto m : members_of_v<T>) {
            using F = [:std::meta::type_of(m):];

            if constexpr (OptionalLike<F>) {
                if (what.[:m:].has_value())
                    json_value<F>::write(out[json_key_of<m>()], what.[:m:]);
            } else
                json_value<F>::write(out[json_key_of<m>()], what.[:m:]);
        }
    }
};
