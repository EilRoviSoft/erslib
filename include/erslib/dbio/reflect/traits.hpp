#pragma once

// std
#include <string>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/optional.hpp>


// Declaration

namespace dbio::impl::reflect {
    template<typename T>
    struct sql_type;
}


// Implementation

namespace dbio::impl::reflect {
    template<>
    struct sql_type<bool> {
        static constexpr std::string_view name = "BOOLEAN";
        static constexpr bool nullable = false;
    };

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


namespace dbio::impl::reflect {
    template<>
    struct sql_type<std::string> {
        static constexpr std::string_view name = "TEXT";
        static constexpr bool nullable = false;
    };

    template<typename T>
    struct sql_type<ers::optional<T>> : sql_type<T> {
        static constexpr bool nullable = true;
    };
}
