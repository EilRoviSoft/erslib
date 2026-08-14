#include "erslib/dbio/layouts/update.hpp"

// ers
#include <erslib/dbio/clauses/assign.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/slots/set.hpp>
#include <erslib/dbio/slots/where.hpp>


using namespace dbio::impl;


// Impl

namespace {
    ////&slots::from,
    ////&slots::returning

    constexpr SlotBinding update_layout[] = {
        {
            .slot   = &slots::update,
            .prefix = "UPDATE "
        },
        {
            .slot      = &slots::set,
            .prefix    = "\nSET ",
            .separator = ", "
        },
        {
            .slot      = &slots::where,
            .prefix    = "\nWHERE ",
            .separator = " AND "
        },
    };
}


// Public API

dbio::impl::Query dbio::impl::layouts::update(std::string table) {
    Query q(update_layout);
    q |= clauses::identifier(&slots::update, std::move(table));
    return q;
}
