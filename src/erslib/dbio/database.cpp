#include "erslib/dbio/database.hpp"

// std
#include <format>

// ers
#include <erslib/core/type/error.hpp>
#include <erslib/dbio/query_store.hpp>


// db_options_t

namespace dbio {
    std::string db_options_t::connection_string() const {
        return std::format("host={} port={} dbname={} user={} password={}",
            host, port, dbname, username, password);
    }
}


// Database

dbio::Database::Database(const std::string& connection_string, QueryStore queries) :
    m_connection(connection_string),
    m_queries(std::move(queries)) {
}


ers::Status dbio::Database::init(const fs::path& root, std::span<const std::string> query_labels) try {
    pqxx::work tnx(m_connection);

    for (const auto& label : query_labels) {
        auto query = m_queries.get(label);
        if (!query)
            continue;

        pqxx::subtransaction subtnx(tnx);
        subtnx.exec(*query);
        subtnx.commit();
    }

    tnx.commit();
    return ers::ok;
} catch (const pqxx::sql_error& e) {
    return ers::make_error(ers::Severity::Error, "dbio: schema init failed: {}", e.what());
} catch (const pqxx::usage_error& e) {
    return ers::make_error(ers::Severity::Error, "dbio: schema init failed: {}", e.what());
}


dbio::Database dbio::make_database(const db_options_t& options, QueryStore queries) {
    return Database(options.connection_string(), std::move(queries));
}
