#pragma once

// std
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <vector>

// pqxx
#include <pqxx/transaction>

// ers
#include <erslib/core/filesystem.hpp>
#include <erslib/core/memory.hpp>
#include <erslib/core/trait/fn.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/core/type/time.hpp>
#include <erslib/dbio/impl/options.hpp>
#include <erslib/dbio/impl/query_builder.hpp>


namespace dbio::impl {
    struct pool_options_t;

    template<typename Fn>
    concept ConnectionHandler = std::invocable<Fn, pqxx::connection&>;

    template<typename Fn>
    using transaction_arg_t = std::remove_cvref_t<
        typename ers::fn_traits<std::remove_cvref_t<Fn>>::template arg_type<0>
    >;

    template<typename Fn>
    concept TransactionCallable = requires { typename transaction_arg_t<Fn>; }
        && std::derived_from<transaction_arg_t<Fn>, pqxx::dbtransaction>;
}


namespace dbio::impl {
    class ConnectionPool : public ers::enable_shared_from_this<ConnectionPool> {
        friend class Connection;


    public:
        class Connection;


        // Member functions

        ConnectionPool(std::string db_connection_string, pool_options_t pool_opts);


        // Modifiers

        ers::Result<Connection> acquire();
        Connection acquire_or_throw();

        void maintain();


    private:
        struct entry_t {
            pqxx::connection* conn;
            ers::timestamp_t last_used;
        };


        bool _is_stale(const entry_t& entry, ers::timestamp_t now) const noexcept;


        void _init();

        void _release(pqxx::connection* conn) noexcept;
        // mutex should acquired already
        void _discard(pqxx::connection* conn) noexcept;

        // mutex should acquired already
        ers::Result<pqxx::connection*> _spawn();


        std::string _db_connection_string;
        pool_options_t _pool_opts;

        mutable std::mutex _mutex;
        ers::file_ptr _trace_file;
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
        Connection(ers::shared_ptr<ConnectionPool> pool, pqxx::connection* conn) noexcept;


        void _release() noexcept;


        ers::shared_ptr<ConnectionPool> _pool;
        pqxx::connection* _conn;
    };
}
