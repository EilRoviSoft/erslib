#include "erslib/conduit/clauses/group.hpp"

// ers
#include <erslib/conduit/clauses/target.hpp>
#include <erslib/conduit/slots/group.hpp>


conduit::impl::Clause conduit::impl::clauses::group_by(std::string column) {
    return identifier(&slots::group, std::move(column));
}
