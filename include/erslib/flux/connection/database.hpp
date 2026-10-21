#pragma once

// std
#include <string>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/flux/connection/connection_pool.hpp>
#include <erslib/flux/connection/options.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT Database {
    public:
        using Connection = ConnectionPool::Connection;


        // Member functions

        explicit Database(std::string connection_string, pool_options_t pool_opts = {});


        // Accessors

        ConnectionPool& pool() { return *_pool; }
        const ConnectionPool& pool() const { return *_pool; }


        // Connections

        ers::Result<Connection> acquire() { return _pool->acquire(); }
        Connection acquire_or_throw() { return _pool->acquire_or_throw(); }


        // Modifiers

        void maintain();


    private:
        std::shared_ptr<ConnectionPool> _pool;
    };
}
