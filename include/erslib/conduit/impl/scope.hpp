#pragma once

// std
#include <string_view>
#include <type_traits>
#include <utility>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/conduit/impl/connection_pool.hpp>
#include <erslib/conduit/impl/query_builder.hpp>


namespace conduit::impl {
    template<typename T>
    concept ConnectionSource = requires(T& source) {
        { source.acquire_or_throw() } -> std::same_as<ConnectionPool::Connection>;
    };


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

    template<TransactionCallable Fn>
    auto with_tx(pqxx::connection& conn, Fn&& fn, std::string_view name = {}) {
        using Tx = transaction_arg_t<Fn>;
        using return_type = std::invoke_result_t<Fn, Tx&>;

        Tx tx(conn, name);

        if constexpr (std::is_void_v<return_type>) {
            std::forward<Fn>(fn)(tx);
            tx.commit();
        } else {
            auto result = std::forward<Fn>(fn)(tx);
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
    auto with_query(Source& source, std::string_view name, Q query) {
        auto conn = source.acquire_or_throw();
        Tx tx(*conn, name);

        auto result = with_query(tx, std::move(query));
        tx.commit();

        return result;
    }

    template<typename Tx = pqxx::work, ConnectionSource Source, Executable Q>
    auto with_query(Source& source, Q query) {
        return with_query<Tx>(source, std::string_view {}, std::move(query));
    }
}
