#include "erslib/dbio/layouts/delete.hpp"

// ers
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/slots/where.hpp>


// Impl

namespace {
    constexpr dbio::impl::SlotRef delete_layout[] = {
        &dbio::impl::slots::delete_from,
        &dbio::impl::slots::where,
    };
}


// Public API

dbio::impl::Query dbio::impl::layouts::delete_from(std::string table) {
    Query q(delete_layout);
    q |= clauses::identifier(&slots::delete_from, std::move(table));
    return q;
}
