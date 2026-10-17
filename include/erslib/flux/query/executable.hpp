#pragma once

// std
#include <concepts>
#include <type_traits>

// pqxx
#include <pqxx/dbtransaction>

// ers
#include <erslib/flux/result/query_result.hpp>


namespace flux::impl {
    template<typename T>
    concept Executable = requires(const T& q, pqxx::dbtransaction& tx) {
        { q.exec(tx) } -> std::same_as<QueryResult>;
    };

    template<typename T>
    concept ExecutableWithOutput = Executable<T>
        && requires { typename T::Output; }
        && !std::is_void_v<typename T::Output>;
}
