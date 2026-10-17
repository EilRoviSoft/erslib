#include "erslib/flux/layouts/delete.hpp"

// ers
#include <erslib/flux/clauses/target.hpp>
#include <erslib/flux/clauses/where.hpp>
#include <erslib/flux/slots/returning.hpp>
#include <erslib/flux/slots/where.hpp>


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
