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


// Impl

namespace {
    ////&slots::distinct,
    ////&slots::group,
    ////&slots::having,
    ////&slots::window,
    ////&slots::locking

    constexpr dbio::impl::SlotBinding select_layout[] = {
        {
            .slot      = &dbio::impl::slots::column,
            .prefix    = "SELECT ",
            .separator = ", ",
            .fallback  = "*"
        },
        dbio::impl::bindings::from,
        dbio::impl::bindings::where,
        dbio::impl::bindings::order,
        dbio::impl::bindings::limit,
        dbio::impl::bindings::offset,
    };
}


// Public API

dbio::impl::Query dbio::impl::layouts::select_from(std::string table) {
    Query q(select_layout);
    q |= clauses::from(std::move(table));
    return q;
}
