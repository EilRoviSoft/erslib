#include "erslib/dbio/options.hpp"

// std
#include <format>


std::string dbio::db_options_t::connection_string() const {
    return std::format("host={} port={} dbname={} user={} password={}",
        host, port, dbname, username, password);
}
