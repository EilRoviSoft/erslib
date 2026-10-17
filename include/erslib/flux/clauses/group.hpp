#pragma once

// std
#include <string>

// ers
#include <erslib/flux/impl/clause.hpp>


namespace flux::impl::clauses {
    Clause group_by(std::string column);
}
