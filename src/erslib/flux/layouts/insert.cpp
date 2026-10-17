#include "erslib/flux/layouts/insert.hpp"

// ers
#include <erslib/flux/clauses/conflict.hpp>
#include <erslib/flux/clauses/target.hpp>
#include <erslib/flux/clauses/values.hpp>
#include <erslib/flux/slots/column.hpp>
#include <erslib/flux/slots/conflict.hpp>
#include <erslib/flux/slots/returning.hpp>
#include <erslib/flux/slots/values.hpp>


using namespace flux::impl;


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
