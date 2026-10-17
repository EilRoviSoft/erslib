#include "erslib/flux/impl/database.hpp"

// ers
#include <erslib/flux/impl/query_store.hpp>


flux::impl::Database::Database(std::string connection_string, pool_options_t pool_opts) :
    _pool(ers::make_shared<ConnectionPool>(std::move(connection_string), std::move(pool_opts))) {
}

void flux::impl::Database::maintain() {
    _pool->maintain();
}
