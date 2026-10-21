#pragma once

// Goal is to have something like this:
// auto users = (select_from("users")
//     | columns("id", "name")
//     | where("guild_id", guild_id)
//     | order_by("created_at", EOrder::Desc)
//     | with_limit(20)
// ).exec(tx).get_as<std::vector<User>>();


// Includes --------------------------------------------------------------------

#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/expr/node.hpp>
#include <erslib/flux/expr/ops.hpp>
#include <erslib/flux/query/clause.hpp>
#include <erslib/flux/query/layout.hpp>
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/query/slot.hpp>

// Clauses

#include <erslib/flux/query/clauses/aggregate.hpp>
#include <erslib/flux/query/clauses/assign.hpp>
#include <erslib/flux/query/clauses/bound.hpp>
#include <erslib/flux/query/clauses/conditional.hpp>
#include <erslib/flux/query/clauses/conflict.hpp>
#include <erslib/flux/query/clauses/distinct.hpp>
#include <erslib/flux/query/clauses/exists.hpp>
#include <erslib/flux/query/clauses/expr.hpp>
#include <erslib/flux/query/clauses/group.hpp>
#include <erslib/flux/query/clauses/identity.hpp>
#include <erslib/flux/query/clauses/join.hpp>
#include <erslib/flux/query/clauses/order.hpp>
#include <erslib/flux/query/clauses/raw.hpp>
#include <erslib/flux/query/clauses/returning.hpp>
#include <erslib/flux/query/clauses/values.hpp>
#include <erslib/flux/query/clauses/where.hpp>
#include <erslib/flux/query/clauses/where_any.hpp>
#include <erslib/flux/query/clauses/where_between.hpp>
#include <erslib/flux/query/clauses/where_column.hpp>
#include <erslib/flux/query/clauses/where_expr.hpp>
#include <erslib/flux/query/clauses/where_in.hpp>
#include <erslib/flux/query/clauses/where_null.hpp>
#include <erslib/flux/query/clauses/where_op.hpp>
#include <erslib/flux/query/clauses/where_or.hpp>

// Layouts

#include <erslib/flux/query/layouts/delete.hpp>
#include <erslib/flux/query/layouts/exists.hpp>
#include <erslib/flux/query/layouts/insert.hpp>
#include <erslib/flux/query/layouts/select.hpp>
#include <erslib/flux/query/layouts/update.hpp>

// Slots

#include <erslib/flux/query/slots/column.hpp>
#include <erslib/flux/query/slots/conflict.hpp>
#include <erslib/flux/query/slots/distinct.hpp>
#include <erslib/flux/query/slots/from.hpp>
#include <erslib/flux/query/slots/group.hpp>
#include <erslib/flux/query/slots/join.hpp>
#include <erslib/flux/query/slots/limit.hpp>
#include <erslib/flux/query/slots/offset.hpp>
#include <erslib/flux/query/slots/order.hpp>
#include <erslib/flux/query/slots/returning.hpp>
#include <erslib/flux/query/slots/set.hpp>
#include <erslib/flux/query/slots/values.hpp>
#include <erslib/flux/query/slots/where.hpp>


// Exports ---------------------------------------------------------------------

namespace flux {
    namespace string_literals {
        using impl::operator ""_op;
    }

    namespace orm {
        using namespace impl::clauses;
        using namespace impl::layouts;
        using namespace string_literals;

        using impl::EOrder;
        using impl::EOp;
    }

    namespace clauses = impl::clauses;
    namespace layouts = impl::layouts;
    namespace slots = impl::slots;
    namespace bindings = impl::bindings;
}


namespace flux {
    using impl::Expr;

    using impl::QueryBuilder;
    using impl::QueryResult;

    using impl::WithCte;
    using impl::WithUnion;


    using impl::to_string;
    using impl::to_op;
    using impl::operator ""_op;
}


namespace flux::ext {
    using impl::IExprNode;
    using impl::EPrecedence;
    using impl::binary_op_t;
    using impl::fn_t;
    using impl::sql_text;
    using impl::make_binary;
    using impl::make_fn;
    using impl::make_raw;


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

    using impl::OrderClause;

    using impl::RawClause;

    using impl::IdentityClause;

    using impl::ValuesClause;

    using impl::IWhereClause;
    using impl::WhereAnyClause;
    using impl::WhereBetweenClause;
    using impl::WhereColumnClause;
    using impl::WhereExprClause;
    using impl::WhereInClause;
    using impl::WhereNullClause;
    using impl::WhereOpClause;
    using impl::WhereOrClause;
}


// Macros ----------------------------------------------------------------------

#define ERS_FLUX_WITH_ORM using namespace flux::orm
