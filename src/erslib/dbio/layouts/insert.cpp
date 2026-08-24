#include "erslib/dbio/layouts/insert.hpp"

// ers
#include <erslib/dbio/clauses/conflict.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/values.hpp>
#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/conflict.hpp>
#include <erslib/dbio/slots/returning.hpp>
#include <erslib/dbio/slots/values.hpp>


using namespace dbio::impl;


// Impl

namespace {
    constexpr SlotBinding insert_layout[] = {
        {
            .slot   = &slots::insert_into,
            .prefix = "INSERT INTO "
        },
        {
            .slot      = &slots::column,
            .prefix    = " (",
            .separator = ", ",
            .suffix    = ")"
        },
        bindings::values,
        bindings::conflict,
        bindings::conflict_constraint,
        bindings::conflict_action,
        bindings::returning,
    };
}


// Public API

QueryBuilder layouts::insert_into(std::string table) {
    QueryBuilder q(insert_layout);
    q |= clauses::identifier(&slots::insert_into, std::move(table));
    return q;
}
