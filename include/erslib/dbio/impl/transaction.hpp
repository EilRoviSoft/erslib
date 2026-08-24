#pragma once

// std
#include <string_view>
#include <type_traits>
#include <utility>

// pqxx
#include <pqxx/transaction>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/connection_pool.hpp>


// Callable wrappers
//
// Out-of-line definitions for ConnectionPool's template members.
// Kept apart  the pool itself so the RAII/pooling half stays readable;
// include this header (or impl/database.hpp) whenever you call with_connection / with_transaction / call.

namespace dbio::impl {
    template<typename Fn>
        requires ConnectionHandler<Fn>
    auto ConnectionPool::with_connection(Fn&& fn) -> ers::flattened_result_t<std::invoke_result_t<Fn, pqxx::connection&>> {
        using return_type = std::invoke_result_t<Fn, pqxx::connection&>;


        auto conn = acquire();
        if (!conn)
            return conn.error();

        try {
            if constexpr (std::is_void_v<return_type>) {
                std::forward<Fn>(fn)(*conn.value());
                return ers::ok;
            } else {
                return std::forward<Fn>(fn)(*conn.value());
            }
        } catch (const pqxx::failure& e) {
            return ers::make_error("connection failed: {}", e.what());
        }
    }

    template<typename Fn>
        requires TransactionCallable<Fn>
    auto ConnectionPool::with_transaction(
        Fn&& fn, std::string_view name
    ) -> ers::flattened_result_t<std::invoke_result_t<Fn, transaction_arg_t<Fn>&>> {
        using Tx = transaction_arg_t<Fn>;
        using return_type = std::invoke_result_t<Fn, Tx&>;


        auto conn = acquire();
        if (!conn)
            return conn.error();

        try {
            Tx tx(*conn.value(), name);

            if constexpr (std::is_void_v<return_type>) {
                std::forward<Fn>(fn)(tx);
                tx.commit();
                return ers::ok;
            } else {
                auto result = std::forward<Fn>(fn)(tx);
                tx.commit();
                return result;
            }
        } catch (const pqxx::failure& e) {
            return ers::make_error("transaction failed: {}", e.what());
        }
    }

    template<typename Tx, typename DbioFn, typename... Args>
        requires std::derived_from<Tx, pqxx::dbtransaction> && std::invocable<DbioFn, Tx&, Args...>
    auto ConnectionPool::call(DbioFn&& fn, Args&&... args) {
        return with_transaction([&fn, &args...](Tx& tx) -> decltype(auto) {
            return std::invoke(std::forward<DbioFn>(fn), tx, std::forward<Args>(args)...);
        });
    }
}
