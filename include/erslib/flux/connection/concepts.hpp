#pragma once

// std
#include <concepts>
#include <type_traits>

// pqxx
#include <pqxx/connection>
#include <pqxx/dbtransaction>

// ers
#include <erslib/core/trait/fn.hpp>
#include <erslib/flux/connection/connection_pool.hpp>


namespace flux::impl {
    template<typename Fn>
    concept ConnectionHandler = std::invocable<Fn, pqxx::connection&>;

    template<typename Fn>
    using transaction_arg_t = std::remove_cvref_t<
        typename ers::fn_traits<std::remove_cvref_t<Fn>>::template arg_type<0>
    >;

    template<typename Fn>
    concept TransactionCallable = requires { typename transaction_arg_t<Fn>; }
        && std::derived_from<transaction_arg_t<Fn>, pqxx::dbtransaction>;


    template<typename T>
    concept ConnectionSource = requires(T& source) {
        { source.acquire_or_throw() } -> std::same_as<ConnectionPool::Connection>;
    };
}
