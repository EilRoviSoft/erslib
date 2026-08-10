#pragma once

// std
#include <chrono>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// pqxx
#include <pqxx/connection>

// ers
#include <erslib/core/trait/fn.hpp>
#include <erslib/core/trait/result.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/time.hpp>
#include <erslib/dbio/database.hpp>
#include <erslib/dbio/fwd.hpp>


namespace dbio {
    template<typename Fn>
    concept ConnectionHandler = std::invocable<Fn, pqxx::connection&>;

    template<typename T>
    using flattened_result_t = std::conditional_t<ers::is_result_v<T>, T, ers::Result<T>>;

    template<typename Fn>
    using transaction_arg_t = std::remove_cvref_t<
        typename ers::fn_traits<std::remove_cvref_t<Fn>>::template arg_type<0>
    >;

    template<typename Fn>
    concept TransactionCallable =
        requires { typename transaction_arg_t<Fn>; }
        && std::derived_from<transaction_arg_t<Fn>, pqxx::dbtransaction>;
}


namespace dbio {
    struct pool_options_t {
        size_t min_size = 1;
        size_t max_size = 4;
        ms_t acquire_timeout { 5000 };
        ms_t idle_timeout { 1000 };
    };


    class ConnectionPool : public std::enable_shared_from_this<ConnectionPool> {
        friend class Connection;

    public:
        class Connection;


        // Member functions

        ConnectionPool(std::string db_opts, pool_options_t pool_opts);


        // Modifiers

        ers::Result<Connection> acquire();

        void maintain();


        template<typename Fn>
            requires ConnectionHandler<Fn>
        auto with_connection(Fn&& fn) -> ers::Result<std::invoke_result_t<Fn, pqxx::connection&>>;

        template<typename Fn>
            requires TransactionCallable<Fn>
        auto with_transaction(Fn&& fn, std::string_view name = {}) -> flattened_result_t<std::invoke_result_t<Fn, transaction_arg_t<Fn>&>>;

        template<typename Tx = pqxx::work, typename DbioFn, typename... Args>
            requires std::derived_from<Tx, pqxx::dbtransaction> && std::invocable<DbioFn, Tx&, Args...>
        auto call(DbioFn&& fn, Args&&... args);


    private:
        struct entry_t {
            pqxx::connection* conn;
            ers::timestamp_t last_used;
        };


        void _init();

        void _release(pqxx::connection* conn) noexcept;
        // mutex should acquired already
        void _discard(pqxx::connection* conn) noexcept;

        // mutex should acquired already
        pqxx::connection* _spawn();


        std::string_view _db_connection_string;
        pool_options_t _pool_opts;

        mutable std::mutex _mutex;
        std::condition_variable _cv;
        std::vector<std::unique_ptr<pqxx::connection>> _all;
        std::deque<entry_t> _idle;
    };


    class ConnectionPool::Connection {
        friend class ConnectionPool;

    public:
        // Member functions

        Connection(Connection&& other) noexcept;
        Connection& operator=(Connection&& other) noexcept;

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;


        ~Connection();


        pqxx::connection& operator*() noexcept;
        pqxx::connection* operator->() noexcept;


    private:
        Connection(std::shared_ptr<ConnectionPool> pool, pqxx::connection* conn) noexcept;


        void _release() noexcept;


        std::shared_ptr<ConnectionPool> _pool;
        pqxx::connection* _conn;
    };
}


namespace dbio {
    template<typename Fn>
        requires ConnectionHandler<Fn>
    auto ConnectionPool::with_connection(Fn&& fn) -> ers::Result<std::invoke_result_t<Fn, pqxx::connection&>> {
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
    ) -> flattened_result_t<std::invoke_result_t<Fn, transaction_arg_t<Fn>&>> {
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
        return with_transaction([&](Tx& tx) -> decltype(auto) {
            return fn(tx, args...);
        });
    }
}
