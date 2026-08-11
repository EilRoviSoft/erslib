#pragma once

// std
#include <cstdint>
#include <string>

// ers
#include <erslib/dbio/fwd.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
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

    struct pool_options_t {
        size_t min_size = 1;
        size_t max_size = 4;
        ms_t acquire_timeout { 5000 };
        ms_t idle_timeout { 1000 };
    };
}


// Exports

namespace dbio {
    using impl::db_options_t;
    using impl::pool_options_t;
}
