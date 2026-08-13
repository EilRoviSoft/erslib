#include "erslib/dbio/database.hpp"

// std
#include <format>

// ers
#include <erslib/core/type/diagnostic.hpp>
#include <erslib/dbio/query_store.hpp>


dbio::impl::Database::Database(std::string connection_string, pool_options_t pool_opts) :
    _pool(ers::make_shared<ConnectionPool>(std::move(connection_string), std::move(pool_opts))) {
}

ers::Result<dbio::impl::Database::Connection> dbio::impl::Database::acquire() {
    return _pool->acquire();
}

ers::Status dbio::impl::Database::init(std::string_view label) {
    auto query = queries.get(label);
    if (!query)
        return ers::make_warning("Query '{}' doesn't exist.", label);

    return with_transaction([&query](pqxx::work& tx) -> ers::Status {
        tx.exec(*query);
        tx.commit();
        return ers::ok;
    }, "Init");
}

void dbio::impl::Database::maintain() {
    _pool->maintain();
}
