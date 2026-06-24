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

namespace dbio {
    Database::Database(const db_options_t& options) :
        m_connection(options.connection_string()) {
    }

    Database::Database(const std::string& connection_string) :
        m_connection(connection_string) {
    }

    pqxx::connection& Database::connection() noexcept {
        return this->m_connection;
    }
    const pqxx::connection& Database::connection() const noexcept {
        return this->m_connection;
    }

    ers::Status Database::init(const QueryStore& queries, std::span<const std::string> query_labels) try {
        pqxx::work tnx(m_connection);

        for (const auto& label : query_labels) {
            auto query = queries.get(label);
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
}
