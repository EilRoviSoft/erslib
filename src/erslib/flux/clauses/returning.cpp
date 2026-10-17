#include "erslib/flux/clauses/returning.hpp"

// ers
#include <erslib/flux/clauses/raw.hpp>
#include <erslib/flux/clauses/target.hpp>
#include <erslib/flux/slots/returning.hpp>


flux::impl::Clause flux::impl::clauses::returning(std::string name) {
    return identifier(&slots::returning, std::move(name));
}

flux::impl::Clause flux::impl::clauses::returning_all() {
    return raw(&slots::returning, "*");
}
