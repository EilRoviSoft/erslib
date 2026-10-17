#pragma once

// std
#include <string>

// ers
#include <erslib/flux/query/clause.hpp>


namespace flux::impl::clauses {
    Clause group_by(std::string column);
}
