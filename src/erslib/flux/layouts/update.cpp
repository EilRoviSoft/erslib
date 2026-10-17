#include "erslib/flux/layouts/update.hpp"

// ers
#include <erslib/flux/clauses/assign.hpp>
#include <erslib/flux/clauses/target.hpp>
#include <erslib/flux/clauses/where.hpp>
#include <erslib/flux/slots/returning.hpp>
#include <erslib/flux/slots/set.hpp>
#include <erslib/flux/slots/where.hpp>


using namespace flux::impl;


// Impl

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


// Public API

QueryBuilder layouts::update(std::string table) {
    QueryBuilder q(update_layout);
    q |= clauses::identifier(&slots::update, std::move(table));
    return q;
}
