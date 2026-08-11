#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/query.hpp>


namespace dbio {
    namespace clauses {
        Query select_from(std::string table);
    }
}
