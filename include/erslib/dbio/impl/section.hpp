#pragma once

// std
#include <cstdint>
#include <string_view>

// export
#include <erslib/export.hpp>


namespace dbio {
    using Section = uint32_t;

    namespace section {
        inline constexpr Section select = 100;      // "SELECT <column>, ..."
        inline constexpr Section insert_into = 150; // "INSERT INTO <table>"
        inline constexpr Section column = 300;      // either "*", or "<column>, ..."
        inline constexpr Section source = 400;      // "FROM <source>"
        inline constexpr Section where = 500;       // "WHERE <cond>"
        inline constexpr Section order_by = 600;    // "ORDER BY <cond>"
        inline constexpr Section values = 700;      // "VALUES (...), (...)"
        inline constexpr Section limit = 800;       // "LIMIT <n>"
        inline constexpr Section offset = 900;      // "OFFSET <n>"
    }
}

namespace dbio::internal {
    struct section_format_t {
        std::string_view prefix;
        std::string_view separator;
        std::string_view suffix;
    };


    ERSLIB_EXPORT section_format_t section_format(Section section);

    ERSLIB_EXPORT bool is_singular(Section section);
    ERSLIB_EXPORT bool is_identifier(std::string_view name);
}
