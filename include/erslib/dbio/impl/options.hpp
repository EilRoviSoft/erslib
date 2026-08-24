#pragma once

// std
#include <string>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/time.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    struct ERSLIB_EXPORT db_options_t {
        std::string host = "localhost";
        u16 port = 5432;
        std::string dbname;
        std::string username;
        std::string password;


        // libpqxx key/value connection string built from the fields above.
        [[nodiscard]]
        std::string connection_string() const;
    };

    struct pool_options_t {
        size_t min_size = 1;
        size_t max_size = 4;
        ers::ms_t acquire_timeout = std::chrono::seconds(5);
        ers::ms_t idle_timeout = std::chrono::minutes(5);
    };
}
