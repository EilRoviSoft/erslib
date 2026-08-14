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
    constexpr dbio::impl::SlotRef select_layout[] = {
        &dbio::impl::slots::column,
        //&dbio::impl::slots::distinct,
        &dbio::impl::slots::from,
        &dbio::impl::slots::where,
        //&dbio::impl::slots::group,
        //&dbio::impl::slots::having,
        //&dbio::impl::slots::window,
        &dbio::impl::slots::order,
        &dbio::impl::slots::limit,
        &dbio::impl::slots::offset,
        //&dbio::impl::slots::locking
    };
}


// Public API

dbio::impl::Query dbio::impl::layouts::select_from(std::string table) {
    Query q(select_layout);
    q |= clauses::from(std::move(table));
    return q;
}
