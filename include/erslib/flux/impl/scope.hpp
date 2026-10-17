#pragma once

// std
#include <string_view>
#include <type_traits>
#include <utility>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/flux/impl/connection_pool.hpp>
#include <erslib/flux/impl/query_builder.hpp>
#include <erslib/core/trait/result.hpp>


// Definitions

namespace flux::impl {
    template<typename T>
    concept ConnectionSource = requires(T& source) {
        { source.acquire_or_throw() } -> std::same_as<ConnectionPool::Connection>;
    };
}


// Throwing counterparts

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


// Fallible query result

namespace flux::impl {
    class TryQueryResult;
}


template<>
struct ers::impl::as_result<flux::impl::QueryResult> {
    using type = flux::impl::TryQueryResult;
};


// Try-catch counterparts

namespace flux::impl {
    template<std::invocable Fn>
    auto try_invoke(Fn&& fn, std::string_view name = {}) -> ers::as_result_t<std::invoke_result_t<Fn>> {
        using R = std::invoke_result_t<Fn>;

        const auto label = name.empty() ? std::string_view("database call") : name;

        try {
            if constexpr (std::is_void_v<R>) {
                std::invoke(std::forward<Fn>(fn));
                return ers::ok;
            } else
                return std::invoke(std::forward<Fn>(fn));
        } catch (const pqxx::broken_connection& e) {
            return ers::make_crit("'{}': connection lost: {}", label, ers::util::trim(e.what()));
        } catch (const pqxx::sql_error& e) {
            return ers::make_error("'{}' failed [{}]: {}", label, e.sqlstate(), ers::util::trim(e.what()));
        } catch (const std::exception& e) {
            return ers::make_error("'{}' failed: {}", label, e.what());
        }
    }


    class TryQueryResult : public ers::Result<QueryResult> {
    public:
        using ers::Result<QueryResult>::Result;


        template<typename T>
        ers::Result<T> get_as() const {
            if (!*this)
                return error();

            return try_invoke([&] { return value().template get_as<T>(); });
        }

        template<ReadableRow T>
        ers::Result<RowGenerator<T>> view_as() const {
            if (!*this)
                return error();

            return try_invoke([&] { return value().template view_as<T>(); });
        }
    };


    template<ConnectionSource Source, ConnectionHandler Fn>
    auto try_with_conn(Source& source, Fn&& fn) {
        return try_invoke([&] { return with_conn(source, std::forward<Fn>(fn)); });
    }

    template<ConnectionSource Source, TransactionCallable Fn>
    auto try_with_tx(Source& source, Fn&& fn, std::string_view name = {}) {
        return try_invoke([&] { return with_tx(source, std::forward<Fn>(fn), name); }, name);
    }

    template<TransactionCallable Fn>
    auto try_with_tx(pqxx::connection& conn, Fn&& fn, std::string_view name = {}) {
        return try_invoke([&] { return with_tx(conn, std::forward<Fn>(fn), name); }, name);
    }

    template<typename Tx = pqxx::work, ConnectionSource Source, Executable Q>
    auto try_with_query(Source& source, std::string_view name, Q query) {
        return try_invoke([&] { return with_query<Tx>(source, name, std::move(query)); }, name);
    }

    template<typename Tx = pqxx::work, ConnectionSource Source, Executable Q>
    auto try_with_query(Source& source, Q query) {
        return try_with_query<Tx>(source, std::string_view {}, std::move(query));
    }
}
