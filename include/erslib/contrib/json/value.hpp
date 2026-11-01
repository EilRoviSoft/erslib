#pragma once

// std
#include <concepts>
#include <string>
#include <string_view>

// ers
#include <erslib/contrib/json/impl.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>


// Traits ----------------------------------------------------------------------

namespace utl::impl {
    template<typename T>
    struct json_value;

    template<typename Node>
    concept JsonNode = json_detail::is_node_v<Node>;

    template<typename T>
    concept JsonConvertible = requires(const Json& json, Json& out, const T& what) {
        { json_value<T>::read(json) } -> std::same_as<ers::Result<T>>;
        json_value<T>::write(out, what);
    };
}


// Helpers ---------------------------------------------------------------------

namespace utl::impl {
    template<JsonNode Node>
    auto expected_error(std::string_view what, const Node& got) {
        return ers::make_error("expected {}, got {}",
            what, type_label(got.type()));
    }

    template<typename S>
    std::string utf8_of(const S& text) {
        return json_detail::to_string_type<std::string>(text);
    }

    template<typename T>
    ers::Result<T> missing_field(std::string_view key) {
        if constexpr (OptionalLike<T>)
            return T {};
        else
            return ers::make_error("no field '{}'", key);
    }
}


// Functions -------------------------------------------------------------------

namespace utl::impl {
    template<typename T, JsonNode Node>
        requires JsonConvertible<T>
    ers::Result<T> read(const Node& json) {
        return json_value<T>::read(json);
    }

    template<typename T, JsonNode Node>
        requires JsonConvertible<T>
    ers::Result<T> read(const Node& json, std::string_view key) {
        if (!json.is_object())
            return missing_field<T>(key);

        auto node = json.try_at(key);

        if (!node)
            return missing_field<T>(key);

        return json_value<T>::read(*node)
            .context("'{}'", key);
    }

    template<JsonNode Node = Json, typename T>
        requires JsonConvertible<T>
    Node write(const T& what) {
        Node out;
        json_value<T>::write(out, what);
        return out;
    }

    template<JsonNode Node, typename T>
        requires JsonConvertible<T>
    void write(Node& out, const T& what) {
        json_value<T>::write(out, what);
    }
}
