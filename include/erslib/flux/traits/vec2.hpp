#pragma once

// std
#include <charconv>
#include <format>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/flux/sql/value.hpp>


template<typename T>
struct flux::impl::sql_type<vec2<T>> {
    static constexpr std::string_view name = "POINT";
    static constexpr bool nullable = false;
};

template<typename T>
struct flux::impl::sql_value<vec2<T>> {
    static void bind(pqxx::params& out, const vec2<T>& what) {
        out.append(std::format("({},{})", what.x, what.y));
    }

    static vec2<T> read(const auto& field) {
        static constexpr vec2<T> fallback = { .x = 0, .y = 0 };
        const std::string_view text = field.template as<std::string_view>();

        vec2<T> result;

        if (!text.starts_with('(') || !text.ends_with(')'))
            return fallback;

        size_t offset;

        if (auto r = std::from_chars(text.begin() + 1, text.end(), result.x)) {
            offset = r.ptr - text.data();
        } else
            return fallback;

        if (offset >= text.size() || text[offset] != ',')
            return fallback;

        offset++;
        if (!std::from_chars(text.begin() + offset, text.end(), result.y))
            return fallback;

        return result;
    }
};
