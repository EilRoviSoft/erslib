#pragma once

// std
#include <string>

// ers
#include <erslib/flux/query/clause.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl::clauses {
    ERSLIB_EXPORT Clause group_by(std::string column);
}
