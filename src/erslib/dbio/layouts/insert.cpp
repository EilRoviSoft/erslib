#include "erslib/dbio/layouts/insert.hpp"

// ers
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/values.hpp>
#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/values.hpp>


// Impl

namespace {
    constexpr dbio::impl::SlotBinding insert_layout[] = {
        {
            .slot   = &dbio::impl::slots::insert_into,
            .prefix = "INSERT INTO "
        },
        {
            .slot      = &dbio::impl::slots::column,
            .prefix    = " (",
            .separator = ", ",
            .suffix    = ")"
        },
        dbio::impl::bindings::values,
    };
}


// Public API

dbio::impl::Query dbio::impl::layouts::insert_into(std::string table) {
    Query q(insert_layout);
    q |= clauses::identifier(&slots::insert_into, std::move(table));
    return q;
}
