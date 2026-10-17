#pragma once

// std
#include <array>
#include <charconv>
#include <concepts>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

// ers
#include <erslib/core/type/fixed_string.hpp>


namespace flux::impl::reflect {
    template<ers::fixed_string Sql>
    struct raw_sql_t {
        static constexpr std::string_view sql = Sql.to_sv();
    };

    template<ers::fixed_string Sql>
    inline constexpr raw_sql_t<Sql> raw_sql {};

    template<typename T>
    concept RawSql = requires {
        { T::sql } -> std::convertible_to<std::string_view>;
    };
}

namespace flux::impl::reflect {
    template<auto Value>
    consteval std::string sql_literal() {
        using type = std::remove_cvref_t<decltype(Value)>;

        if constexpr (RawSql<type>) {
            return std::string(type::sql);
        } else if constexpr (std::is_same_v<type, bool>) {
            return Value ? "TRUE" : "FALSE";
        } else if constexpr (std::is_integral_v<type>) {
            char buffer[32] {};
            const auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), Value);
            return std::string(buffer, ptr);
        } else {
            static_assert(
                requires { Value.to_sv(); },
                "Default<> supports bool, integral, fixed_string and raw_sql values only."
            );

            std::string out = "'";

            for (const char it : Value.to_sv()) {
                if (it == '\'')
                    out += '\'';

                out += it;
            }

            out += '\'';
            return out;
        }
    }

    consteval std::string qualify(std::string_view schema, std::string_view table) {
        if (schema.empty() || table.contains('.'))
            return std::string(table);

        return std::string(schema) + '.' + std::string(table);
    }

    template<std::ranges::input_range R>
    consteval std::string join(const R& what, std::string_view separator) {
        std::string out;
        bool first = true;

        for (const auto& it : what) {
            if (!first)
                out += separator;

            out += it;
            first = false;
        }

        return out;
    }

    consteval std::string field_list(std::span<const std::string_view> what) {
        return '(' + join(what, ", ") + ')';
    }
}
