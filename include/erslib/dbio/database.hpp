#pragma once

// std
#include <filesystem>
#include <string>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/query_store.hpp>
#include <erslib/dbio/connection_pool.hpp>

// export
#include <erslib/export.hpp>


namespace fs = std::filesystem;


// db_options_t

namespace dbio {
    struct ERSLIB_EXPORT db_options_t {
        std::string host = "localhost";
        std::uint16_t port = 5432;
        std::string dbname;
        std::string username;
        std::string password;


        // libpqxx key/value connection string built from the fields above.
        [[nodiscard]]
        std::string connection_string() const;
    };
}


// Database

namespace dbio {
    // Thin owning wrapper around a pqxx::connection.
    class ERSLIB_EXPORT Database {
    public:
        using Connection = ConnectionPool::Connection;


        // Member functions

        explicit Database(std::string connection_string, pool_options_t pool_opts = {});


        // Accessors

        ers::Result<Connection> acquire();


        template<typename Fn>
            requires ConnectionHandler<Fn>
        auto with_connection(Fn&& fn) {
            return _pool->with_connection(std::forward<Fn>(fn));
        }

        template<typename Fn>
            requires TransactionCallable<Fn>
        auto with_transaction(Fn&& fn, std::string_view name = {}) {
            return _pool->with_transaction(std::forward<Fn>(fn), name);
        }

        template<typename Tx = pqxx::work, typename DbioFn, typename... Args>
        auto call(DbioFn&& fn, Args&&... args) {
            return _pool->call<Tx>(std::forward<DbioFn>(fn), std::forward<Args>(args)...);
        }


        // Initializers

        // Runs each query found in the given store under the provided labels, every one in its own savepoint.
        // Missing labels are skipped.
        // Intended for schema bootstrap (e.g. the generated CREATE TABLE statements).
        ers::Status init(pqxx::dbtransaction& tx, const QueryStore& queries, std::string_view label);


        // Modifiers

        void maintain();


    private:
        std::shared_ptr<ConnectionPool> _pool;
    };
}
