#pragma once

// std
#include <string>

// pqxx
#include <pqxx/util>

// ers
#include <erslib/core/type/general.hpp>


namespace dbio::impl {
    enum class Mode : u8 { HexLower, HexUpper, Text };

    std::string bytes_to_string(const pqxx::bytes& bytes, std::string_view args);
}


// Exports

namespace dbio {
    using impl::Mode;
    using impl::bytes_to_string;
}
