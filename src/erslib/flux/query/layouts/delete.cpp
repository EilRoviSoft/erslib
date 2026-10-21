#include "erslib/flux/query/layouts/delete.hpp"

// std
#include <utility>

// ers
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/clauses/where.hpp>
#include <erslib/flux/query/slots/returning.hpp>
#include <erslib/flux/query/slots/where.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

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


// Implementation --------------------------------------------------------------

flux::impl::QueryBuilder flux::impl::layouts::delete_from(std::string table) {
    QueryBuilder q(delete_layout);
    q |= clauses::identifier(&slots::delete_from, std::move(table));
    return q;
}
