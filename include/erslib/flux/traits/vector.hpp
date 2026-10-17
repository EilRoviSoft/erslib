#pragma once

// std
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/flux/result/row.hpp>
#include <erslib/flux/sql/value.hpp>


namespace flux::impl {
    template<typename T>
    inline constexpr bool is_vector_v = false;

    template<typename T>
    inline constexpr bool is_vector_v<std::vector<T>> = true;

    template<typename T>
    inline constexpr bool is_optional_v = false;

    template<typename T>
    inline constexpr bool is_optional_v<ers::optional<T>> = true;
}


template<typename T>
struct flux::impl::sql_value<std::vector<T>> {
    static std::string to_text(const std::vector<T>& what) {
        std::string literal = "{";

        for (size_t i = 0; i < what.size(); i++) {
            if (i)
                literal += ',';

            _append(literal, what[i]);
        }

        return literal += '}';
    }

    static void bind(pqxx::params& out, const std::vector<T>& what) {
        out.append(to_text(what));
    }


private:
    template<typename U>
    static void _append(std::string& out, const U& what) {
        if constexpr (is_optional_v<U>) {
            if (what)
                _append(out, *what);
            else
                out += "NULL";
        } else if constexpr (std::is_arithmetic_v<U> || is_vector_v<U>)
            out += sql_value<U>::to_text(what);
        else
            _append_quoted(out, sql_value<U>::to_text(what));
    }

    static void _append_quoted(std::string& out, std::string_view what) {
        out += '"';

        for (const char c : what) {
            if (c == '"' || c == '\\')
                out += '\\';

            out += c;
        }

        out += '"';
    }
};
