#pragma once

// std
#include <concepts>
#include <format>
#include <string>
#include <utility>

// pqxx
#include <pqxx/params>

// ers
#include <erslib/flux/sql/binder.hpp>


namespace flux::impl {
    template<typename T>
    struct sql_type;
}


namespace flux::impl {
    template<typename T>
    struct sql_value {
        static std::string to_text(const T& what) {
            return std::format("{}", what);
        }

        static void bind(pqxx::params& out, const T& what) {
            out.append(what);
        }

        static T read(const auto& field) {
            return field.template as<T>();
        }
    };


    template<typename T>
    binder_t make_binder(T&& value) {
        return [stored = owned_t<T>(std::forward<T>(value))](pqxx::params& out) {
            sql_value<owned_t<T>>::bind(out, stored);
        };
    }


    template<typename T>
    concept HasSqlText = requires(const T& v) {
        { sql_value<T>::to_text(v) } -> std::convertible_to<std::string>;
    };

    template<typename T>
    void append_array_element(std::string& out, const T& what) {
        const std::string text = [&] {
            if constexpr (HasSqlText<T>)
                return sql_value<T>::to_text(what);
            else
                return std::format("{}", what);
        }();

        out += '"';

        for (const char c : text) {
            if (c == '"' || c == '\\')
                out += '\\';

            out += c;
        }

        out += '"';
    }
}
