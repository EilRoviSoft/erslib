#include "erslib/conduit/layouts/delete.hpp"

// ers
#include <erslib/conduit/clauses/target.hpp>
#include <erslib/conduit/clauses/where.hpp>
#include <erslib/conduit/slots/returning.hpp>
#include <erslib/conduit/slots/where.hpp>


using namespace conduit::impl;


// Impl

namespace {
    ////&slots::using,

    constexpr SlotBinding delete_layout[] = {
        {
            .slot   = &slots::delete_from,
            .prefix = "DELETE FROM "
        },
        bindings::where,
        bindings::returning,
    };
}


// Public API

conduit::impl::QueryBuilder conduit::impl::layouts::delete_from(std::string table) {
    QueryBuilder q(delete_layout);
    q |= clauses::identifier(&slots::delete_from, std::move(table));
    return q;
}
