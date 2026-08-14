#include "erslib/dbio/layouts/update.hpp"

// ers
#include <erslib/dbio/clauses/assign.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/slots/set.hpp>
#include <erslib/dbio/slots/where.hpp>


// Impl

namespace {
    constexpr dbio::impl::SlotRef update_layout[] = {
        &dbio::impl::slots::update,
        &dbio::impl::slots::set,
        &dbio::impl::slots::where,
    };
}


// Public API

dbio::impl::Query dbio::impl::layouts::update(std::string table) {
    Query q(update_layout);
    q |= clauses::identifier(&slots::update, std::move(table));
    return q;
}
