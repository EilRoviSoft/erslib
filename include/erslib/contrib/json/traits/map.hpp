#pragma once

// std
#include <string_view>

// ers
#include <erslib/contrib/json/value.hpp>


template<utl::impl::MapLike M>
    requires utl::impl::StringLike<typename M::key_type>
struct utl::impl::json_value<M> {
    using mapped = typename M::mapped_type;


    template<typename Policy, JsonNode Node>
    static read_t<Policy, M> read(const Node& json) {
        if (!json.is_object())
            return wrong_type<M, Policy>("object", json);

        M out;

        for (const auto& [key, node] : json.as_object()) {
            auto name = utf8_of(key);
            auto item = json_value<mapped>::template read<Policy>(node);

            if (!item)
                return Policy::template propagate<M>(std::move(item), "'{}'", name);

            out.try_emplace(typename M::key_type(name), std::move(*item));
        }

        return out;
    }

    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, const M& what) {
        out = typename Node::object_type {};

        for (const auto& [key, value] : what) {
            auto name = utf8_of(key);

            auto status = json_value<mapped>::template write<Policy>(out[std::string_view(name)], value);

            if (!status)
                return Policy::template propagate<void>(std::move(status), "'{}'", name);
        }

        return Policy::done();
    }
};
