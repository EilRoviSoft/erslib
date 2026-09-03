#pragma once

// Goal is to have something like this:
// auto users = (select_from("users")
//     | columns("id", "name")
//     | where("guild_id", guild_id)
//     | order_by("created_at", EOrder::Desc)
//     | with_limit(20)
// ).exec(tx).get_as<std::vector<User>>();


// Main implementation

#include <erslib/conduit/impl/clause.hpp>
#include <erslib/conduit/impl/layout.hpp>
#include <erslib/conduit/impl/query_builder.hpp>
#include <erslib/conduit/impl/slot.hpp>

// Clauses

#include <erslib/conduit/clauses/aggregate.hpp>
#include <erslib/conduit/clauses/assign.hpp>
#include <erslib/conduit/clauses/bound.hpp>
#include <erslib/conduit/clauses/conditional.hpp>
#include <erslib/conduit/clauses/conflict.hpp>
#include <erslib/conduit/clauses/dinstinct.hpp>
#include <erslib/conduit/clauses/exists.hpp>
#include <erslib/conduit/clauses/expr.hpp>
#include <erslib/conduit/clauses/group.hpp>
#include <erslib/conduit/clauses/join.hpp>
#include <erslib/conduit/clauses/order.hpp>
#include <erslib/conduit/clauses/raw.hpp>
#include <erslib/conduit/clauses/returning.hpp>
#include <erslib/conduit/clauses/target.hpp>
#include <erslib/conduit/clauses/values.hpp>
#include <erslib/conduit/clauses/where.hpp>
#include <erslib/conduit/clauses/where_any.hpp>
#include <erslib/conduit/clauses/where_between.hpp>
#include <erslib/conduit/clauses/where_column.hpp>
#include <erslib/conduit/clauses/where_in.hpp>
#include <erslib/conduit/clauses/where_null.hpp>
#include <erslib/conduit/clauses/where_op.hpp>
#include <erslib/conduit/clauses/where_or.hpp>

// Layouts

#include <erslib/conduit/layouts/delete.hpp>
#include <erslib/conduit/layouts/exists.hpp>
#include <erslib/conduit/layouts/insert.hpp>
#include <erslib/conduit/layouts/select.hpp>
#include <erslib/conduit/layouts/update.hpp>

// Slots

#include <erslib/conduit/slots/column.hpp>
#include <erslib/conduit/slots/conflict.hpp>
#include <erslib/conduit/slots/dinstinct.hpp>
#include <erslib/conduit/slots/from.hpp>
#include <erslib/conduit/slots/group.hpp>
#include <erslib/conduit/slots/join.hpp>
#include <erslib/conduit/slots/limit.hpp>
#include <erslib/conduit/slots/offset.hpp>
#include <erslib/conduit/slots/order.hpp>
#include <erslib/conduit/slots/returning.hpp>
#include <erslib/conduit/slots/set.hpp>
#include <erslib/conduit/slots/values.hpp>
#include <erslib/conduit/slots/where.hpp>


// Exports

namespace conduit {
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


namespace conduit {
    using impl::QueryBuilder;
    using impl::QueryResult;

    using impl::WithCte;
    using impl::WithUnion;


    using impl::EOp;
    using impl::to_string;
    using impl::to_op;
    using impl::operator ""_op;
}


namespace conduit::ext {
    using impl::binder_t;
    using impl::make_binder;

    using impl::Context;

    using impl::Layout;

    using impl::EArity;
    using impl::Slot;
    using impl::SlotRef;
    using impl::SlotBinding;
}

namespace conduit::ext {
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

    using impl::EOrder;
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
