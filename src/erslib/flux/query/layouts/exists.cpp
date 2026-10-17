#include "erslib/flux/query/layouts/exists.hpp"

// ers
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/slots/exists.hpp>
#include <erslib/flux/query/slots/where.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

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


// Implementation --------------------------------------------------------------

QueryBuilder layouts::exists_in(std::string table) {
    QueryBuilder q(exists_layout);
    q |= clauses::identifier(&slots::exists_target, std::move(table));
    return q;
}
