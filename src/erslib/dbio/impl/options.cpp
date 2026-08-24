#include "erslib/dbio/options.hpp"

// std
#include <format>

// ers
#include <erslib/core/util/string.hpp>


std::string dbio::impl::db_options_t::connection_string() const {
    std::string out = std::format("host={} port={}", ers::util::quote(host), port);

    auto append = [&](std::string_view key, std::string_view value) {
        if (value.empty())
            return;

        out += ' ';
        out += key;
        out += '=';
        out += ers::util::quote(value);
    };

    append("dbname", dbname);
    append("user", username);
    append("password", password);

    return out;
}
