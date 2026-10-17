#include "erslib/flux/query/layouts/insert.hpp"

// ers
#include <erslib/flux/query/clauses/conflict.hpp>
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/clauses/values.hpp>
#include <erslib/flux/query/slots/column.hpp>
#include <erslib/flux/query/slots/conflict.hpp>
#include <erslib/flux/query/slots/returning.hpp>
#include <erslib/flux/query/slots/values.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

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


// Implementation --------------------------------------------------------------

QueryBuilder layouts::insert_into(std::string table) {
    QueryBuilder q(insert_layout);
    q |= clauses::identifier(&slots::insert_into, std::move(table));
    return q;
}
