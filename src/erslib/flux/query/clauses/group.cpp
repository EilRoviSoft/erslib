#include "erslib/flux/query/clauses/group.hpp"

// ers
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/slots/group.hpp>


flux::impl::Clause flux::impl::clauses::group_by(std::string column) {
    return identifier(&slots::group, std::move(column));
}
