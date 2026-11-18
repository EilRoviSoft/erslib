#pragma once

// std
#include <format>
#include <optional>
#include <string>
#include <string_view>
#include <variant>

// ers
#include <erslib/contrib/json/annotations.hpp>
#include <erslib/contrib/json/value.hpp>
#include <erslib/core/type/fixed_string.hpp>


// Type wrapper ----------------------------------------------------------------

namespace utl::impl {
    template<ers::fixed_string Tag, typename... Ts>
    concept JsonVariantAlternatives = (sizeof...(Ts) > 0)
        && (JsonObjectType<Ts> && ...)
        && (has_json_tag<Ts>() && ...)
        && json_tags_distinct<Ts...>()
        && json_types_distinct<Ts...>()
        && (!has_json_key<Ts>(Tag.view()) && ...);


    template<ers::fixed_string Tag, typename... Ts>
        requires JsonVariantAlternatives<Tag, Ts...>
    struct json_variant : std::variant<Ts...> {
        using variant_type = std::variant<Ts...>;

        using variant_type::variant;
        using variant_type::operator=;

        static constexpr std::string_view tag_key = Tag.view();
        static constexpr size_t size = sizeof...(Ts);
    };


    template<typename T>
    inline constexpr bool is_json_variant_v = false;

    template<ers::fixed_string Tag, typename... Ts>
    inline constexpr bool is_json_variant_v<json_variant<Tag, Ts...>> = true;

    template<typename T>
    concept JsonVariant = is_json_variant_v<T>;
}


// I/O -------------------------------------------------------------------------

template<ers::fixed_string Tag, typename... Ts>
struct utl::impl::json_value<utl::impl::json_variant<Tag, Ts...>> {
    using T = json_variant<Tag, Ts...>;


    static std::string known_tags() {
        std::string out;

        ((out += out.empty() ? "" : ", ", out += json_tag_of<Ts>()), ...);

        return out;
    }

    template<typename Policy, JsonNode Node, typename Alt>
    static bool try_alternative(const Node& json, std::string_view name, std::optional<read_t<Policy, T>>& out) {
        if (name != json_tag_of<Alt>())
            return false;

        auto value = json_value<Alt>::template read<Policy>(json);

        if (!value)
            out.emplace(Policy::template propagate<T>(std::move(value), "{}", json_tag_of<Alt>()));
        else
            out.emplace(T(std::in_place_type<Alt>, std::move(*value)));

        return true;
    }


    template<typename Policy, JsonNode Node>
    static read_t<Policy, T> read(const Node& json) {
        if (!json.is_object())
            return wrong_type<T, Policy>("object", json);

        auto node = json.try_at(Tag.view());

        if (!node) {
            return Policy::template fail<T>([&] {
                return std::format("no field '{}'", Tag.view());
            });
        }

        if (!node->is_string()) {
            return Policy::template propagate<T>(
                wrong_type<std::string, Policy>("string", *node), "'{}'", Tag.view());
        }

        auto name = utf8_of(node->as_string());

        std::optional<read_t<Policy, T>> out;

        (try_alternative<Policy, Node, Ts>(json, name, out) || ...);

        if (out)
            return std::move(*out);

        return Policy::template fail<T>([&] {
            return std::format("unknown '{}' value '{}', expected one of: {}", Tag.view(), name, known_tags());
        });
    }


    template<typename Policy, JsonNode Node>
    static write_t<Policy> write(Node& out, const T& what) {
        if (what.valueless_by_exception()) {
            return Policy::template fail<void>([] {
                return std::string("the variant holds no value");
            });
        }

        return std::visit([&]<typename Alt>(const Alt& alt) -> write_t<Policy> {
            auto status = json_value<Alt>::template write<Policy>(out, alt);

            if (!status)
                return Policy::template propagate<void>(std::move(status), "{}", json_tag_of<Alt>());

            out[Tag.view()] = json_tag_of<Alt>();

            return Policy::done();
        }, static_cast<const typename T::variant_type&>(what));
    }
};


// Formatter -------------------------------------------------------------------

#ifdef ERSLIB_HAS_REFLECTION

template<ers::fixed_string Tag, typename... Ts>
struct std::formatter<utl::impl::json_variant<Tag, Ts...>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();

        if (it != ctx.end() && *it != '}')
            throw ers::format_error("Invalid json_variant format spec");

        return it;
    }

    auto format(const utl::impl::json_variant<Tag, Ts...>& value, std::format_context& ctx) const {
        if (value.valueless_by_exception())
            return std::format_to(ctx.out(), "<valueless>");

        return std::visit([&](const auto& alt) {
            return std::format_to(ctx.out(), "{}", alt);
        }, static_cast<const typename utl::impl::json_variant<Tag, Ts...>::variant_type&>(value));
    }
};

#endif
