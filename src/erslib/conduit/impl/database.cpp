#include "erslib/conduit/impl/database.hpp"

// ers
#include <erslib/conduit/impl/query_store.hpp>


conduit::impl::Database::Database(std::string connection_string, pool_options_t pool_opts) :
    _pool(ers::make_shared<ConnectionPool>(std::move(connection_string), std::move(pool_opts))) {
}

void conduit::impl::Database::maintain() {
    _pool->maintain();
}
