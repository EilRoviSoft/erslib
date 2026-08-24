#include "erslib/dbio/layouts/select.hpp"

// ers
#include <erslib/dbio/clauses/bound.hpp>
#include <erslib/dbio/clauses/order.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/from.hpp>
#include <erslib/dbio/slots/limit.hpp>
#include <erslib/dbio/slots/offset.hpp>
#include <erslib/dbio/slots/order.hpp>
#include <erslib/dbio/slots/where.hpp>


using namespace dbio::impl;


// Impl

namespace {
    ////&slots::distinct,
    ////&slots::group,
    ////&slots::having,
    ////&slots::window,
    ////&slots::locking

    constexpr SlotBinding select_layout[] = {
        {
            .slot      = &slots::column,
            .prefix    = "SELECT ",
            .separator = ", ",
            .fallback  = "*"
        },
        bindings::from,
        bindings::where,
        bindings::order,
        bindings::limit,
        bindings::offset,
    };
}


// Public API

QueryBuilder layouts::select_from(std::string table) {
    QueryBuilder q(select_layout);
    q |= clauses::from(std::move(table));
    return q;
}
