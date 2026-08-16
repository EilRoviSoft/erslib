#pragma once

// std
#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>


// Extension point for C++ type -> SQL type.
//
//     template<>
//     struct dbio::reflect::sql_type_traits<my::Money> {
//         static constexpr std::string_view value = "NUMERIC(12, 2)";
//     };

namespace dbio::reflect {
    template<typename T>
    struct sql_type_traits;


    template<typename T>
    concept HasSqlType = requires {
        { sql_type_traits<T>::value } -> std::convertible_to<std::string_view>;
    };

    template<typename T>
    concept SqlNullable = HasSqlType<T> && requires {
        { sql_type_traits<T>::nullable } -> std::convertible_to<bool>;
    };

    template<typename T>
    consteval bool is_nullable() {
        if constexpr (SqlNullable<T>)
            return sql_type_traits<T>::nullable;
        else
            return false;
    }
}


// Built-in mappings

namespace dbio::reflect {
    template<> struct sql_type_traits<bool>     { static constexpr std::string_view value = "BOOLEAN"; };

    template<> struct sql_type_traits<int16_t>  { static constexpr std::string_view value = "SMALLINT"; };
    template<> struct sql_type_traits<int32_t>  { static constexpr std::string_view value = "INTEGER"; };
    template<> struct sql_type_traits<int64_t>  { static constexpr std::string_view value = "BIGINT"; };


    // Postgres has no unsigned types, so widen.

    template<> struct sql_type_traits<uint16_t> { static constexpr std::string_view value = "INTEGER"; };
    template<> struct sql_type_traits<uint32_t> { static constexpr std::string_view value = "BIGINT"; };
    template<> struct sql_type_traits<uint64_t> { static constexpr std::string_view value = "NUMERIC(20)"; };


    template<> struct sql_type_traits<float>    { static constexpr std::string_view value = "REAL"; };
    template<> struct sql_type_traits<double>   { static constexpr std::string_view value = "DOUBLE PRECISION"; };


    template<> struct sql_type_traits<std::string>      { static constexpr std::string_view value = "TEXT"; };
    template<> struct sql_type_traits<std::string_view> { static constexpr std::string_view value = "TEXT"; };


    // Wrappers delegate: a mapping for T covers optional<T>.
    template<HasSqlType T>
    struct sql_type_traits<std::optional<T>> {
        static constexpr std::string_view value = sql_type_traits<T>::value;
        static constexpr bool nullable = true;
    };
}
