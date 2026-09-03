#include "erslib/conduit/layouts/update.hpp"

// ers
#include <erslib/conduit/clauses/assign.hpp>
#include <erslib/conduit/clauses/target.hpp>
#include <erslib/conduit/clauses/where.hpp>
#include <erslib/conduit/slots/returning.hpp>
#include <erslib/conduit/slots/set.hpp>
#include <erslib/conduit/slots/where.hpp>


using namespace conduit::impl;


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
