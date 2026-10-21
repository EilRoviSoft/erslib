#include "erslib/flux/connection/database.hpp"

// std
#include <utility>


flux::impl::Database::Database(std::string connection_string, pool_options_t pool_opts) :
    _pool(std::make_shared<ConnectionPool>(std::move(connection_string), std::move(pool_opts))) {
}

void flux::impl::Database::maintain() {
    _pool->maintain();
}
