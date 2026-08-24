#include "erslib/dbio/layouts/update.hpp"

// ers
#include <erslib/dbio/clauses/assign.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/slots/returning.hpp>
#include <erslib/dbio/slots/set.hpp>
#include <erslib/dbio/slots/where.hpp>


using namespace dbio::impl;


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
