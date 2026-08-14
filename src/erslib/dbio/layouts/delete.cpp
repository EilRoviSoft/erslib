#include "erslib/dbio/layouts/delete.hpp"

// ers
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/slots/where.hpp>


using namespace dbio::impl;


// Impl

namespace {
    ////&slots::using,
    ////&slots::returning

    constexpr SlotBinding delete_layout[] = {
        {
            .slot   = &slots::delete_from,
            .prefix = "DELETE FROM "
        },
        {
            .slot      = &slots::where,
            .prefix    = "\nWHERE ",
            .separator = " AND "
        },
    };
}


// Public API

dbio::impl::Query dbio::impl::layouts::delete_from(std::string table) {
    Query q(delete_layout);
    q |= clauses::identifier(&slots::delete_from, std::move(table));
    return q;
}
