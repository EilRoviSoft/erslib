#pragma once

// std
#include <meta>
#include <string>
#include <string_view>

// pqxx
#include <pqxx/params>

// ers
#include <erslib/contrib/json.hpp>
#include <erslib/flux/sql/value.hpp>


namespace flux {
    struct jsonb_t {};
    inline constexpr jsonb_t jsonb {};

    template<typename T>
    concept JsonbColumn = std::is_class_v<T>
        && !std::meta::annotations_of_with_type(^^T, ^^jsonb_t).empty()
        && utl::JsonConvertible<T>;
}

template<flux::JsonbColumn T>
struct flux::impl::sql_type<T> {
    static constexpr std::string_view name = "JSONB";
    static constexpr bool nullable = false;
};

template<flux::JsonbColumn T>
struct flux::impl::sql_value<T> {
    static void bind(pqxx::params& out, const T& what) {
        out.append(utl::write(what).to_string(utl::EFormat::Minimized));
    }

    static T read(const auto& field) {
        auto json = utl::try_from_string(field.template as<std::string_view>());

        if (!json)
            throw ers::runtime_error(json.error().to_string());

        return utl::read<T>(*json)
            .context("jsonb column")
            .template or_throw<ers::runtime_error>();
    }
};
