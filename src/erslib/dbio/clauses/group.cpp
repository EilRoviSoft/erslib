#include "erslib/dbio/clauses/group.hpp"

// ers
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/slots/group.hpp>


dbio::impl::Clause dbio::impl::clauses::group_by(std::string column) {
    return identifier(&slots::group, std::move(column));
}
