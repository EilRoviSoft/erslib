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
    concept DeducibleTransaction = requires {
        typename transaction_arg_t<Fn>;
    };


    template<typename Fn>
    concept TransactionCallable = std::derived_from<transaction_arg_t<Fn>, pqxx::dbtransaction> && requires {
        typename transaction_arg_t<Fn>;
    };
}

namespace flux::impl {
    template<typename Fn>
    struct tx_type_of {
        using type = pqxx::work;
    };

    template<DeducibleTransaction Fn>
    struct tx_type_of<Fn> {
        using type = transaction_arg_t<Fn>;
    };
}


namespace flux::impl {
    template<typename T>
    concept ConnectionLease = std::movable<T> && requires(T& lease) {
        { *lease } -> std::same_as<pqxx::connection&>;
    };

    template<typename T>
    concept LeaseResult = ers::is_result_v<std::remove_cvref_t<T>>
        && ConnectionLease<typename std::remove_cvref_t<T>::value_type>;
}

namespace flux::impl {
    template<typename T>
    concept ConnectionSource = requires(T& source) {
        { source.acquire_or_throw() } -> ConnectionLease;
    };

    template<typename T>
    concept TryConnectionSource = ConnectionSource<T> && requires(T& source) {
        { source.acquire() } -> LeaseResult;
    };
}
