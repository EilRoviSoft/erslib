#pragma once

// Goal is to have something like this:
// auto users = (select_from("users")
//     | columns("id", "name")
//     | where("guild_id", guild_id)
//     | order_by("created_at", Order::Desc)
//     | with_limit(20)
// ).exec(tx).get_as<std::vector<User>>();


// Main implementation

#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/layout.hpp>
#include <erslib/dbio/impl/query_builder.hpp>
#include <erslib/dbio/impl/slot.hpp>

// Clauses

#include <erslib/dbio/clauses/aggregate.hpp>
#include <erslib/dbio/clauses/assign.hpp>
#include <erslib/dbio/clauses/bound.hpp>
#include <erslib/dbio/clauses/conditional.hpp>
#include <erslib/dbio/clauses/conflict.hpp>
#include <erslib/dbio/clauses/dinstinct.hpp>
#include <erslib/dbio/clauses/exists.hpp>
#include <erslib/dbio/clauses/expr.hpp>
#include <erslib/dbio/clauses/group.hpp>
#include <erslib/dbio/clauses/join.hpp>
#include <erslib/dbio/clauses/order.hpp>
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/clauses/returning.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/values.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/clauses/where_any.hpp>
#include <erslib/dbio/clauses/where_between.hpp>
#include <erslib/dbio/clauses/where_column.hpp>
#include <erslib/dbio/clauses/where_in.hpp>
#include <erslib/dbio/clauses/where_null.hpp>
#include <erslib/dbio/clauses/where_op.hpp>
#include <erslib/dbio/clauses/where_or.hpp>

// Layouts

#include <erslib/dbio/layouts/delete.hpp>
#include <erslib/dbio/layouts/exists.hpp>
#include <erslib/dbio/layouts/insert.hpp>
#include <erslib/dbio/layouts/select.hpp>
#include <erslib/dbio/layouts/update.hpp>

// Slots

#include <erslib/dbio/slots/column.hpp>
#include <erslib/dbio/slots/conflict.hpp>
#include <erslib/dbio/slots/dinstinct.hpp>
#include <erslib/dbio/slots/from.hpp>
#include <erslib/dbio/slots/group.hpp>
#include <erslib/dbio/slots/join.hpp>
#include <erslib/dbio/slots/limit.hpp>
#include <erslib/dbio/slots/offset.hpp>
#include <erslib/dbio/slots/order.hpp>
#include <erslib/dbio/slots/returning.hpp>
#include <erslib/dbio/slots/set.hpp>
#include <erslib/dbio/slots/values.hpp>
#include <erslib/dbio/slots/where.hpp>


// Exports

namespace dbio {
    namespace string_literals {
        using impl::operator ""_op;
    }

    namespace orm {
        using namespace impl::clauses;
        using namespace impl::layouts;
        using namespace string_literals;
    }

    namespace clauses = impl::clauses;
    namespace layouts = impl::layouts;
    namespace slots = impl::slots;
    namespace bindings = impl::bindings;
}


namespace dbio {
    using impl::QueryBuilder;
    using impl::QueryResult;

    using impl::WithCte;
    using impl::WithUnion;


    using impl::Op;
    using impl::to_string;
    using impl::to_op;
    using impl::operator ""_op;
}


namespace dbio::ext {
    using impl::binder_t;
    using impl::make_binder;

    using impl::Context;

    using impl::Layout;

    using impl::Arity;
    using impl::Slot;
    using impl::SlotRef;
    using impl::SlotBinding;
}

namespace dbio::ext {
    using impl::IClause;
    using impl::Clause;


    using impl::AggregateClause;
    using impl::JsonRowsClause;
    using impl::OrderedSetAggClause;
    using impl::StringAggClause;

    using impl::AliasClause;
    using impl::CoalesceClause;
    using impl::SubqueryClause;

    using impl::AssignClause;

    using impl::BoundClause;

    using impl::ConflictUpdateClause;

    using impl::ExistsClause;

    using impl::JoinClause;

    using impl::Order;
    using impl::OrderClause;

    using impl::RawClause;

    using impl::IdentityClause;

    using impl::ValuesClause;

    using impl::IWhereClause;
    using impl::WhereAnyClause;
    using impl::WhereBetweenClause;
    using impl::WhereColumnClause;
    using impl::WhereInClause;
    using impl::WhereNullClause;
    using impl::WhereOpClause;
    using impl::WhereOrClause;
}
