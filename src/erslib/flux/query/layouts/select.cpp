#include "erslib/flux/query/layouts/select.hpp"

// std
#include <utility>

// ers
#include <erslib/flux/query/clauses/bound.hpp>
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/clauses/order.hpp>
#include <erslib/flux/query/clauses/where.hpp>
#include <erslib/flux/query/slots/column.hpp>
#include <erslib/flux/query/slots/distinct.hpp>
#include <erslib/flux/query/slots/from.hpp>
#include <erslib/flux/query/slots/group.hpp>
#include <erslib/flux/query/slots/join.hpp>
#include <erslib/flux/query/slots/limit.hpp>
#include <erslib/flux/query/slots/offset.hpp>
#include <erslib/flux/query/slots/order.hpp>
#include <erslib/flux/query/slots/where.hpp>


using namespace flux::impl;


// Details ---------------------------------------------------------------------

namespace {
    inline constexpr Slot select_kw {
        .name  = "flux.select_kw",
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


// Implementation --------------------------------------------------------------

QueryBuilder layouts::select_from(std::string table, std::string alias) {
    QueryBuilder q(select_layout);
    q |= clauses::from(std::move(table), std::move(alias));
    return q;
}

QueryBuilder layouts::select() {
    return QueryBuilder(select_layout);
}
