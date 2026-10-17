#include "erslib/flux/clauses/group.hpp"

// ers
#include <erslib/flux/clauses/target.hpp>
#include <erslib/flux/slots/group.hpp>


flux::impl::Clause flux::impl::clauses::group_by(std::string column) {
    return identifier(&slots::group, std::move(column));
}
