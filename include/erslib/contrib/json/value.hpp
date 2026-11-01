#pragma once

// std
#include <concepts>
#include <string>
#include <string_view>

// ers
#include <erslib/contrib/json/impl.hpp>
#include <erslib/contrib/json/policy.hpp>
#include <erslib/core/exception.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/core/type/result.hpp>


// Details ---------------------------------------------------------------------

namespace utl::impl {
    template<typename T>
    struct json_value;

    template<typename Node>
    concept JsonNode = json_detail::is_node_v<Node>;

    template<typename T>
    concept JsonConvertible = requires(const Json& json, Json& out, const T& what) {
        { json_value<T>::template read<result_policy>(json) } -> std::same_as<ers::Result<T>>;
        { json_value<T>::template write<result_policy>(out, what) } -> std::same_as<ers::Status>;
    };
}


// Helpers ---------------------------------------------------------------------

namespace utl::impl {
    template<typename S>
    std::string utf8_of(const S& text) {
        return json_detail::to_string_type<std::string>(text);
    }

    template<typename T, typename Policy, JsonNode Node>
    read_t<Policy, T> wrong_type(std::string_view expected, const Node& got) {
        return Policy::template fail<T>([&] {
            return std::format("expected {}, got {}",
                expected, type_label(got.type()));
        });
    }

    template<typename F>
    constexpr bool is_omitted(const F& value) {
        if constexpr (OptionalLike<F>)
            return !value.has_value();
        else
            return false;
    }

    template<typename T, typename Policy, JsonNode Node>
    read_t<Policy, T> read_field(const Node& json, std::string_view key) {
        if (json.is_object()) {
            if (auto node = json.try_at(key)) {
                auto value = json_value<T>::template read<Policy>(*node);

                if (value)
                    return value;

                return Policy::template propagate<T>(std::move(value), "'{}'", key);
            }
        }

        if constexpr (OptionalLike<T>)
            return T {};
        else {
            return Policy::template fail<T>([&] {
                return std::format("no field '{}'", key);
            });
        }
    }
}


// Functions -------------------------------------------------------------------

namespace utl::impl {
    template<typename T, JsonNode Node>
        requires JsonConvertible<T>
    ers::Result<T> try_read(const Node& json) {
        return json_value<T>::template read<result_policy>(json);
    }

    template<typename T, JsonNode Node>
        requires JsonConvertible<T>
    ers::maybe<T> maybe_read(const Node& json) {
        return json_value<T>::template read<maybe_policy>(json);
    }

    template<typename T, JsonNode Node>
        requires JsonConvertible<T>
    T read(const Node& json) {
        return std::move(try_read<T>(json)).template or_throw<ers::conversion_error>();
    }


    template<typename T, JsonNode Node>
        requires JsonConvertible<T>
    ers::Result<T> try_read(const Node& json, std::string_view key) {
        return read_field<T, result_policy>(json, key);
    }

    template<typename T, JsonNode Node>
        requires JsonConvertible<T>
    ers::maybe<T> maybe_read(const Node& json, std::string_view key) {
        return read_field<T, maybe_policy>(json, key);
    }

    template<typename T, JsonNode Node>
        requires JsonConvertible<T>
    T read(const Node& json, std::string_view key) {
        return std::move(try_read<T>(json, key)).template or_throw<ers::conversion_error>();
    }
}


namespace utl::impl {
    template<JsonNode Node, typename T>
        requires JsonConvertible<T>
    ers::Status try_write(Node& out, const T& what) {
        return json_value<T>::template write<result_policy>(out, what);
    }

    template<JsonNode Node, typename T>
        requires JsonConvertible<T>
    bool maybe_write(Node& out, const T& what) {
        return json_value<T>::template write<maybe_policy>(out, what);
    }

    template<JsonNode Node, typename T>
        requires JsonConvertible<T>
    void write(Node& out, const T& what) {
        std::move(try_write(out, what)).template or_throw<ers::conversion_error>();
    }


    template<JsonNode Node = Json, typename T>
        requires JsonConvertible<T>
    ers::Result<Node> try_write(const T& what) {
        Node out;

        if (auto status = try_write(out, what); !status)
            return status.error();

        return out;
    }

    template<JsonNode Node = Json, typename T>
        requires JsonConvertible<T>
    ers::maybe<Node> maybe_write(const T& what) {
        Node out;

        if (!maybe_write(out, what))
            return ers::none;

        return out;
    }

    template<JsonNode Node = Json, typename T>
        requires JsonConvertible<T>
    Node write(const T& what) {
        Node out;
        write(out, what);
        return out;
    }
}
