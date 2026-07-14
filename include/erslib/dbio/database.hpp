#pragma once

// std
#include <filesystem>
#include <span>
#include <string>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/type/result.hpp>

// dbio
#include <erslib/dbio/query_store.hpp>

// export
#include <erslib/export.hpp>


namespace fs = std::filesystem;


// db_options_t

namespace dbio {
    struct ERSLIB_EXPORT db_options_t {
        std::string host = "localhost";
        std::uint16_t port = 5432;
        std::string dbname;
        std::string username;
        std::string password;


        // libpqxx key/value connection string built from the fields above.
        [[nodiscard]]
        std::string connection_string() const;
    };
}


// Database

namespace dbio {
    // Thin owning wrapper around a pqxx::connection.
    class ERSLIB_EXPORT Database {
    public:
        // Member functions

        explicit Database(const std::string& connection_string);


        // Accessors

        pqxx::connection& connection() noexcept { return m_connection; }
        const pqxx::connection& connection() const noexcept { return m_connection; }


        // Initializers

        // Runs each query found in the given store under the provided labels, every one in its own savepoint.
        // Missing labels are skipped.
        // Intended for schema bootstrap (e.g. the generated CREATE TABLE statements).
        ers::Status init(const QueryStore& queries, std::span<const std::string> query_labels);


    protected:
        pqxx::connection m_connection;
    };
}
