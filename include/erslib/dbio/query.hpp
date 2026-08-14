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

#include <erslib/dbio/clauses/general.hpp>
#include <erslib/dbio/clauses/target.hpp>


// Exports

namespace dbio {
    namespace clauses = impl::clauses;


    //using impl::binder_t;
    //using impl::make_binder;

    using impl::ClauseList;
    using impl::ClausePtr;
    using impl::IClause;
    using impl::IdentityClause;

    using impl::Context;

    using impl::Layout;
    using impl::Query;

    using impl::Arity;
    using impl::Slot;
    using impl::SlotRef;
    using impl::SlotView;

    using impl::column_slot;
    using impl::from_slot;
}
