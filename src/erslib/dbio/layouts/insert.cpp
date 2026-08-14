#include "erslib/dbio/layouts/insert.hpp"

// ers
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/values.hpp>
#include <erslib/dbio/slots/values.hpp>


// Impl

namespace {
    constexpr dbio::impl::SlotRef insert_layout[] = {
        &dbio::impl::slots::insert_into,
        &dbio::impl::slots::insert_columns,
        &dbio::impl::slots::values,
    };
}


// Public API

dbio::impl::ClausePtr dbio::impl::clauses::into_column(std::string name) {
    return identifier(&slots::insert_columns, std::move(name));
}

dbio::impl::Query dbio::impl::layouts::insert_into(std::string table) {
    Query q(insert_layout);
    q |= clauses::identifier(&slots::insert_into, std::move(table));
    return q;
}
