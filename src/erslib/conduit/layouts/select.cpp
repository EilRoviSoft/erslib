#include "erslib/conduit/layouts/select.hpp"

// ers
#include <erslib/conduit/clauses/bound.hpp>
#include <erslib/conduit/clauses/order.hpp>
#include <erslib/conduit/clauses/target.hpp>
#include <erslib/conduit/clauses/where.hpp>
#include <erslib/conduit/slots/column.hpp>
#include <erslib/conduit/slots/dinstinct.hpp>
#include <erslib/conduit/slots/from.hpp>
#include <erslib/conduit/slots/group.hpp>
#include <erslib/conduit/slots/join.hpp>
#include <erslib/conduit/slots/limit.hpp>
#include <erslib/conduit/slots/offset.hpp>
#include <erslib/conduit/slots/order.hpp>
#include <erslib/conduit/slots/where.hpp>


using namespace conduit::impl;


// Impl

namespace {
    inline constexpr Slot select_kw {
        .name  = "conduit.select_kw",
        .arity = EArity::Single,
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

QueryBuilder layouts::select() {
    return QueryBuilder(select_layout);
}
