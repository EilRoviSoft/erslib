#include "erslib/conduit/clauses/returning.hpp"

// ers
#include <erslib/conduit/clauses/raw.hpp>
#include <erslib/conduit/clauses/target.hpp>
#include <erslib/conduit/slots/returning.hpp>


conduit::impl::Clause conduit::impl::clauses::returning(std::string name) {
    return identifier(&slots::returning, std::move(name));
}

conduit::impl::Clause conduit::impl::clauses::returning_all() {
    return raw(&slots::returning, "*");
}
