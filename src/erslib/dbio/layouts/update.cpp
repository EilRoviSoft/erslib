#include "erslib/dbio/layouts/update.hpp"

// ers
#include <erslib/dbio/clauses/assign.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/slots/returning.hpp>
#include <erslib/dbio/slots/set.hpp>
#include <erslib/dbio/slots/where.hpp>


// Impl

namespace {
    ////&slots::from,

    constexpr dbio::impl::SlotBinding update_layout[] = {
        {
            .slot   = &dbio::impl::slots::update,
            .prefix = "UPDATE "
        },
        dbio::impl::bindings::set,
        dbio::impl::bindings::where,
        dbio::impl::bindings::returning,
    };
}


// Public API

dbio::impl::QueryBuilder dbio::impl::layouts::update(std::string table) {
    QueryBuilder q(update_layout);
    q |= clauses::identifier(&slots::update, std::move(table));
    return q;
}
