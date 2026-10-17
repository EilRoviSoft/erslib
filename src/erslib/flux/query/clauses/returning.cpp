#include "erslib/flux/query/clauses/returning.hpp"

// ers
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/clauses/raw.hpp>
#include <erslib/flux/query/slots/returning.hpp>


flux::impl::Clause flux::impl::clauses::returning(std::string name) {
    return identifier(&slots::returning, std::move(name));
}

flux::impl::Clause flux::impl::clauses::returning_all() {
    return raw(&slots::returning, "*");
}
