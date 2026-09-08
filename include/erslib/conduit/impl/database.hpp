#pragma once

// std
#include <string>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/conduit/impl/connection_pool.hpp>
#include <erslib/conduit/impl/options.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    class ERSLIB_EXPORT Database {
    public:
        using Connection = ConnectionPool::Connection;


        // Member functions

        explicit Database(std::string connection_string, pool_options_t pool_opts = {});


        // Accessors

        ConnectionPool& pool() { return *_pool; }
        const ConnectionPool& pool() const { return *_pool; }


        // Modifiers

        void maintain();


    private:
        ers::shared_ptr<ConnectionPool> _pool;
    };
}
