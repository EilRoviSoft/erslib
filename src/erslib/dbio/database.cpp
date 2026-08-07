#include "erslib/dbio/database.hpp"

// std
#include <format>

// ers
#include <erslib/core/type/diagnostic.hpp>
#include <erslib/dbio/query_store.hpp>


// db_options_t

namespace dbio {
    std::string db_options_t::connection_string() const {
        return std::format("host={} port={} dbname={} user={} password={}",
            host, port, dbname, username, password);
    }
}


// Database

dbio::Database::Database(const std::string& connection_string) :
    m_connection(connection_string) {
}


ers::Status dbio::Database::init(pqxx::dbtransaction& tx, const QueryStore& queries, std::string_view label) try {
    auto query = queries.get(label);
    if (!query)
        return ers::make_warning("Query '{}' doesn't exist.", label);

    pqxx::subtransaction subtx(tx);
    subtx.exec(*query);
    subtx.commit();

    return ers::ok;
} catch (const pqxx::sql_error& e) {
    return ers::make_error("dbio: schema init failed: {}", e.what());
} catch (const pqxx::usage_error& e) {
    return ers::make_error("dbio: schema init failed: {}", e.what());
}
