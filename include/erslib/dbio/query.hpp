#pragma once

// Goal is to have something like this:
// auto users = (select_from("users")
//     | columns("id", "name")
//     | where("guild_id", guild_id)
//     | order_by("created_at", Order::Desc)
//     | with_limit(20)
// ).exec_as<User>(tx);


// Main implementation

#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/layout.hpp>
#include <erslib/dbio/impl/query.hpp>
#include <erslib/dbio/impl/slot.hpp>

// Clauses

#include <erslib/dbio/clauses/assign.hpp>
#include <erslib/dbio/clauses/bound.hpp>
#include <erslib/dbio/clauses/order.hpp>
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/values.hpp>
#include <erslib/dbio/clauses/where.hpp>

// Layouts
#include <erslib/dbio/layouts/delete.hpp>
#include <erslib/dbio/layouts/insert.hpp>
#include <erslib/dbio/layouts/select.hpp>
#include <erslib/dbio/layouts/update.hpp>

// Slots
#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/from.hpp>
#include <erslib/dbio/slots/limit.hpp>
#include <erslib/dbio/slots/offset.hpp>
#include <erslib/dbio/slots/order.hpp>
#include <erslib/dbio/slots/set.hpp>
#include <erslib/dbio/slots/values.hpp>
#include <erslib/dbio/slots/where.hpp>


// Exports

namespace dbio {
    namespace clauses = impl::clauses;
    namespace layouts = impl::layouts;
    namespace slots = impl::slots;
    namespace bindings = impl::bindings;
}

namespace dbio {
    using impl::binder_t;
    using impl::make_binder;

    using impl::Op;
    using impl::Order;

    using impl::Context;

    using impl::Layout;
    using impl::Query;

    using impl::Arity;
    using impl::Slot;
    using impl::SlotRef;
    using impl::SlotBinding;

    using impl::IClause;
    using impl::Clause;
}

namespace dbio {
    using impl::AssignClause;
    using impl::BoundClause;
    using impl::OrderClause;
    using impl::RawClause;
    using impl::IdentityClause;
    using impl::ValuesClause;

    using impl::IWhereClause;
    using impl::WhereInClause;
    using impl::WhereNullClause;
    using impl::WhereOpClause;
}

#define ERS_QUICK_DBIO_USING \
    using namespace dbio::clauses; \
    using namespace dbio::layouts
