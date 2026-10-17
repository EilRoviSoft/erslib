#pragma once

// Goal is to have something like this:
// auto users = (select_from("users")
//     | columns("id", "name")
//     | where("guild_id", guild_id)
//     | order_by("created_at", EOrder::Desc)
//     | with_limit(20)
// ).exec(tx).get_as<std::vector<User>>();


// Includes --------------------------------------------------------------------

#include <erslib/flux/impl/clause.hpp>
#include <erslib/flux/impl/layout.hpp>
#include <erslib/flux/impl/query_builder.hpp>
#include <erslib/flux/impl/slot.hpp>

// Clauses

#include <erslib/flux/clauses/aggregate.hpp>
#include <erslib/flux/clauses/assign.hpp>
#include <erslib/flux/clauses/bound.hpp>
#include <erslib/flux/clauses/conditional.hpp>
#include <erslib/flux/clauses/conflict.hpp>
#include <erslib/flux/clauses/dinstinct.hpp>
#include <erslib/flux/clauses/exists.hpp>
#include <erslib/flux/clauses/expr.hpp>
#include <erslib/flux/clauses/group.hpp>
#include <erslib/flux/clauses/join.hpp>
#include <erslib/flux/clauses/order.hpp>
#include <erslib/flux/clauses/raw.hpp>
#include <erslib/flux/clauses/returning.hpp>
#include <erslib/flux/clauses/target.hpp>
#include <erslib/flux/clauses/values.hpp>
#include <erslib/flux/clauses/where.hpp>
#include <erslib/flux/clauses/where_any.hpp>
#include <erslib/flux/clauses/where_between.hpp>
#include <erslib/flux/clauses/where_column.hpp>
#include <erslib/flux/clauses/where_in.hpp>
#include <erslib/flux/clauses/where_null.hpp>
#include <erslib/flux/clauses/where_op.hpp>
#include <erslib/flux/clauses/where_or.hpp>

// Layouts

#include <erslib/flux/layouts/delete.hpp>
#include <erslib/flux/layouts/exists.hpp>
#include <erslib/flux/layouts/insert.hpp>
#include <erslib/flux/layouts/select.hpp>
#include <erslib/flux/layouts/update.hpp>

// Slots

#include <erslib/flux/slots/column.hpp>
#include <erslib/flux/slots/conflict.hpp>
#include <erslib/flux/slots/dinstinct.hpp>
#include <erslib/flux/slots/from.hpp>
#include <erslib/flux/slots/group.hpp>
#include <erslib/flux/slots/join.hpp>
#include <erslib/flux/slots/limit.hpp>
#include <erslib/flux/slots/offset.hpp>
#include <erslib/flux/slots/order.hpp>
#include <erslib/flux/slots/returning.hpp>
#include <erslib/flux/slots/set.hpp>
#include <erslib/flux/slots/values.hpp>
#include <erslib/flux/slots/where.hpp>


// Exports ---------------------------------------------------------------------

namespace flux {
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


namespace flux {
    using impl::QueryBuilder;
    using impl::QueryResult;

    using impl::WithCte;
    using impl::WithUnion;


    using impl::EOp;
    using impl::to_string;
    using impl::to_op;
    using impl::operator ""_op;
}


namespace flux::ext {
    using impl::binder_t;
    using impl::make_binder;

    using impl::Context;

    using impl::Layout;

    using impl::EArity;
    using impl::Slot;
    using impl::SlotRef;
    using impl::SlotBinding;
}

namespace flux::ext {
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


// Macros ----------------------------------------------------------------------

#define ERS_FLUX_WITH_ORM using namespace flux::orm
