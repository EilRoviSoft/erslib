#include "erslib/dbio/impl/database.hpp"

// ers
#include <erslib/dbio/impl/query_store.hpp>


dbio::impl::Database::Database(std::string connection_string, pool_options_t pool_opts) :
    _pool(ers::make_shared<ConnectionPool>(std::move(connection_string), std::move(pool_opts))) {
}

ers::Result<dbio::impl::Database::Connection> dbio::impl::Database::acquire() {
    return _pool->acquire();
}

void dbio::impl::Database::maintain() {
    _pool->maintain();
}
