#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/clause.hpp>


namespace dbio::impl::clauses {
    inline Clause group_by(std::string column);
}
