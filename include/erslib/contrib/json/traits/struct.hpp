#pragma once

// ers
#include <erslib/contrib/json/annotations.hpp>
#include <erslib/contrib/json/value.hpp>


template<utl::impl::JsonObjectType T>
struct utl::impl::json_value<T> {
    template<typename Policy, JsonNode Node>
    static read_t<Policy, T> read(const Node& json) {
        if (!json.is_object())
            return wrong_type<T, Policy>("object", json);

        T out {};

        template for (constexpr auto m : members_of_v<T>) {
            if constexpr (!is_json_skipped<m>()) {
                constexpr auto key = json_key_of<m>();
                using F = [:std::meta::type_of(m):];

                auto node = json.try_at(key);

                if (node) {
                    auto value = json_value<F>::template read<Policy>(*node);

                    if (!value)
                        return Policy::template propagate<T>(std::move(value), "'{}'", key);

                    out.[:m:] = std::move(*value);
                } else {
                    if constexpr (!OptionalLike<F>) {
                        return Policy::template fail<T>([&] {
                            return std::format("no field '{}'", key);
                        });
                    }
                }
            }
        }

        return out;
    }

    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, const T& what) {
        out = typename Node::object_type {};

        template for (constexpr auto m : members_of_v<T>) {
            if constexpr (!is_json_skipped<m>()) {
                using F = [:std::meta::type_of(m):];
                constexpr auto key = json_key_of<m>();

                if (!is_omitted(what.[:m:])) {
                    auto status = json_value<F>::template write<Policy>(out[key], what.[:m:]);

                    if (!status)
                        return Policy::template propagate<void>(std::move(status), "'{}'", key);
                }
            }
        }

        return Policy::done();
    }
};
