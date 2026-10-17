#include "erslib/flux/query/layouts/update.hpp"

// ers
#include <erslib/flux/query/clauses/assign.hpp>
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/clauses/where.hpp>
#include <erslib/flux/query/slots/returning.hpp>
#include <erslib/flux/query/slots/set.hpp>
#include <erslib/flux/query/slots/where.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    ////&slots::from,

    constexpr SlotBinding update_layout[] = {
        {
            .slot   = &slots::update,
            .prefix = "UPDATE "
        },
        bindings::set,
        bindings::where,
        bindings::returning,
    };
}


// Implementation --------------------------------------------------------------

QueryBuilder layouts::update(std::string table) {
    QueryBuilder q(update_layout);
    q |= clauses::identifier(&slots::update, std::move(table));
    return q;
}
