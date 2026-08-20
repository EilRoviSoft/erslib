#include "erslib/dbio/clauses/conflict.hpp"


dbio::impl::Clause dbio::impl::clauses::on_conflict_constraint(std::string name) {
    return identifier(&slots::conflict_constraint, std::move(name));
}

dbio::impl::Clause dbio::impl::clauses::do_nothing() {
    return raw(&slots::conflict_action, "DO NOTHING");
}
