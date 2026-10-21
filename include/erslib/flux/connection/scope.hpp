#pragma once

// std
#include <concepts>
#include <string_view>
#include <type_traits>
#include <utility>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/trait/result.hpp>
#include <erslib/flux/connection/concepts.hpp>
#include <erslib/flux/query/executable.hpp>
#include <erslib/flux/result/query_result.hpp>


namespace flux::impl {
    // Transaction level - the caller owns the transaction and its commit.

    template<Executable Q>
        requires (!ExecutableWithOutput<Q>)
    QueryResult with_query(pqxx::dbtransaction& tx, Q query) {
        return query.exec(tx);
    }

    template<ExecutableWithOutput Q>
    typename Q::Output with_query(pqxx::dbtransaction& tx, Q query) {
        return query.exec(tx).template get_as<typename Q::Output>();
    }


    // Connection level - the caller owns the connection.

    template<typename Tx = void, typename Fn>
    auto with_tx(pqxx::connection& conn, Fn&& fn, std::string_view name = {}) {
        using T = std::conditional_t<std::is_void_v<Tx>, typename tx_type_of<Fn>::type, Tx>;
        using return_type = std::invoke_result_t<Fn, T&>;


        static_assert(std::derived_from<T, pqxx::dbtransaction> && !std::is_abstract_v<T>,
            "with_tx: the transaction type must be a concrete pqxx transaction");

        static_assert(std::invocable<Fn, T&>,
            "with_tx: 'fn' can't be called with this transaction");


        T tx(conn, name);

        if constexpr (std::is_void_v<return_type>) {
            std::forward<Fn>(fn)(tx);
            tx.commit();
        } else {
            auto result = std::forward<Fn>(fn)(tx);

            if constexpr (ers::is_result_v<std::remove_cvref_t<return_type>>) {
                if (!result) {
                    tx.abort();
                    return result;
                }
            }

            tx.commit();
            return result;
        }
    }


    // Pool level - acquire, then delegate.

    template<ConnectionSource Source, ConnectionHandler Fn>
    auto with_conn(Source& source, Fn&& fn) {
        auto conn = source.acquire_or_throw();
        return std::forward<Fn>(fn)(*conn);
    }

    template<ConnectionSource Source, TransactionCallable Fn>
    auto with_tx(Source& source, Fn&& fn, std::string_view name = {}) {
        auto conn = source.acquire_or_throw();
        return with_tx(*conn, std::forward<Fn>(fn), name);
    }

    template<typename Tx = pqxx::work, ConnectionSource Source, Executable Q>
    auto with_query(Source& source, Q query, std::string_view name = {}) {
        auto conn = source.acquire_or_throw();
        Tx tx(*conn, name);

        auto result = with_query(tx, std::move(query));
        tx.commit();

        return result;
    }
}
