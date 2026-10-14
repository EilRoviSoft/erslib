#pragma once

// std
#include <concepts>
#include <string_view>
#include <utility>

// ers
#include <erslib/core/type/optional.hpp>

// contrib
#include <erslib/contrib/json/impl.hpp>


namespace utl::impl {
    template<typename T>
    concept JsonReadable = std::same_as<T, Bool>
        || (std::integral<T> && !std::same_as<T, bool>)
        || std::floating_point<T>
        || std::same_as<T, String>
        || std::same_as<T, std::string_view>
        || std::same_as<T, Object>
        || std::same_as<T, Array>;


    template<JsonReadable T>
    ers::optional<T> read_as(const Node& node) {
        if constexpr (std::same_as<T, Bool>) {
            if (const auto* value = node.get_if<Bool>())
                return *value;
        } else if constexpr (std::integral<T>) {
            if (const auto* value = node.get_if<Integral>(); value && std::in_range<T>(*value))
                return static_cast<T>(*value);
        } else if constexpr (std::floating_point<T>) {
            if (const auto* value = node.get_if<Floating>())
                return static_cast<T>(*value);

            if (const auto* value = node.get_if<Integral>())
                return static_cast<T>(*value);
        } else if constexpr (std::same_as<T, std::string_view>) {
            if (const auto* value = node.get_if<String>())
                return std::string_view(*value);
        } else {
            if (const auto* value = node.get_if<T>())
                return *value;
        }

        return ers::nullopt;
    }

    template<JsonReadable T>
    ers::optional<T> read_as(const Node& json, std::string_view key) {
        if (!json.is_object())
            return ers::nullopt;

        if (auto node = json.try_at(key))
            return read_as<T>(*node);

        return ers::nullopt;
    }
}
