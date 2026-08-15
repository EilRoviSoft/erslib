#include "erslib/dbio/clauses/returning.hpp"

// ers
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/slots/returning.hpp>


dbio::impl::Clause dbio::impl::clauses::returning(std::string name) {
    return identifier(&slots::returning, std::move(name));
}

dbio::impl::Clause dbio::impl::clauses::returning_all() {
    return raw(&slots::returning, "*");
}
