#include "erslib/conduit/layouts/insert.hpp"

// ers
#include <erslib/conduit/clauses/conflict.hpp>
#include <erslib/conduit/clauses/target.hpp>
#include <erslib/conduit/clauses/values.hpp>
#include <erslib/conduit/slots/column.hpp>
#include <erslib/conduit/slots/conflict.hpp>
#include <erslib/conduit/slots/returning.hpp>
#include <erslib/conduit/slots/values.hpp>


using namespace conduit::impl;


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
