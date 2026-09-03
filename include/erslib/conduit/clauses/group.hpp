#pragma once

// std
#include <string>

// ers
#include <erslib/conduit/impl/clause.hpp>


namespace conduit::impl::clauses {
    Clause group_by(std::string column);
}
