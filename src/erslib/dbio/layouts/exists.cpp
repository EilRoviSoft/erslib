#include "erslib/dbio/layouts/exists.hpp"

// ers
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/slots/exists.hpp>
#include <erslib/dbio/slots/where.hpp>


using namespace dbio::impl;


// Impl

namespace {
    constexpr SlotBinding exists_layout[] = {
        {
            .slot   = &slots::exists_target,
            .prefix = "SELECT EXISTS (\nSELECT 1 FROM "
        },
        bindings::where,
        {
            .slot     = &slots::exists_alias,
            .prefix   = "\n) AS ",
            .fallback = "\"exists\""
        },
    };
}


// Public API

QueryBuilder layouts::exists_in(std::string table) {
    QueryBuilder q(exists_layout);
    q |= clauses::identifier(&slots::exists_target, std::move(table));
    return q;
}
