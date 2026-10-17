#include "erslib/flux/layouts/select.hpp"

// ers
#include <erslib/flux/clauses/bound.hpp>
#include <erslib/flux/clauses/order.hpp>
#include <erslib/flux/clauses/target.hpp>
#include <erslib/flux/clauses/where.hpp>
#include <erslib/flux/slots/column.hpp>
#include <erslib/flux/slots/dinstinct.hpp>
#include <erslib/flux/slots/from.hpp>
#include <erslib/flux/slots/group.hpp>
#include <erslib/flux/slots/join.hpp>
#include <erslib/flux/slots/limit.hpp>
#include <erslib/flux/slots/offset.hpp>
#include <erslib/flux/slots/order.hpp>
#include <erslib/flux/slots/where.hpp>


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
