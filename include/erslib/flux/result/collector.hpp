#pragma once

// std
#include <concepts>

// pqxx
#include <pqxx/result>


namespace flux::impl {
    template<typename T>
    struct sql_collector;

    template<typename T>
    concept RowContainer = requires(const pqxx::result& r) {
        typename sql_collector<T>::row_t;
        { sql_collector<T>::collect(r) } -> std::same_as<T>;
    };
}
