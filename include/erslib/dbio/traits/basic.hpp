#pragma once

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/dbio/impl/traits.hpp>


namespace dbio::impl {
    template<>
    struct sql_type<i16> {
        static constexpr std::string_view name = "SMALLINT";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<u16> {
        static constexpr std::string_view name = "SMALLINT";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<i32> {
        static constexpr std::string_view name = "INTEGER";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<u32> {
        static constexpr std::string_view name = "INTEGER";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<i64> {
        static constexpr std::string_view name = "BIGINT";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<u64> {
        static constexpr std::string_view name = "BIGINT";
        static constexpr bool nullable = false;
    };
}
