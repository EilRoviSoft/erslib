#pragma once

// std
#include <cstdint>
#include <string>
#include <type_traits>

// ers
#include <erslib/core/type/optional.hpp>


// Declaration

namespace dbio::impl {
    template<typename T>
    struct sql_type;
}


// Implementation

namespace dbio::impl {
    template<>
    struct sql_type<bool> {
        static constexpr std::string_view name = "BOOLEAN";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<int16_t> {
        static constexpr std::string_view name = "SMALLINT";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<uint16_t> {
        static constexpr std::string_view name = "SMALLINT";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<int32_t> {
        static constexpr std::string_view name = "INTEGER";
        static constexpr bool nullable = false;
    };
    
    template<>
    struct sql_type<uint32_t> {
        static constexpr std::string_view name = "INTEGER";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<int64_t> {
        static constexpr std::string_view name = "BIGINT";
        static constexpr bool nullable = false;
    };

    template<>
    struct sql_type<uint64_t> {
        static constexpr std::string_view name = "BIGINT";
        static constexpr bool nullable = false;
    };
}


namespace dbio::impl {
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
