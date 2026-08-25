#include "erslib/dbio/layouts/select.hpp"

// ers
#include <erslib/dbio/clauses/bound.hpp>
#include <erslib/dbio/clauses/order.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/dinstinct.hpp>
#include <erslib/dbio/slots/from.hpp>
#include <erslib/dbio/slots/group.hpp>
#include <erslib/dbio/slots/join.hpp>
#include <erslib/dbio/slots/limit.hpp>
#include <erslib/dbio/slots/offset.hpp>
#include <erslib/dbio/slots/order.hpp>
#include <erslib/dbio/slots/where.hpp>


using namespace dbio::impl;


// Impl

namespace {
    inline constexpr Slot select_kw {
        .name  = "dbio.select_kw",
        .arity = Arity::Single,
    };

    ////&slots::having,
    ////&slots::window,
    ////&slots::locking

    constexpr SlotBinding select_layout[] = {
        {
            .slot     = &select_kw,
            .fallback = "SELECT",
        },
        bindings::distinct,
        {
            .slot      = &slots::column,
            .prefix    = " ",
            .separator = ", ",
            .fallback  = "*",
        },
        bindings::from,
        bindings::join,
        bindings::where,
        bindings::group,
        bindings::order,
        bindings::limit,
        bindings::offset,
    };
}


// Public API

QueryBuilder layouts::select_from(std::string table, std::string alias) {
    QueryBuilder q(select_layout);
    q |= clauses::from(std::move(table), std::move(alias));
    return q;
}
