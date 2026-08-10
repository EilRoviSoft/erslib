#pragma once

// Goal is to have something like this:
// auto users = (from("users")
//     | where("guild_id", guild_id)
//     | where("score", Op::Ge, 100)
//     | order_by("created_at", Order::Desc)
//     | limit(20)
// ).exec_as<User>(tx);


// Main implementation

#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/query.hpp>

// Clauses

#include <erslib/dbio/clauses/bound.hpp>
#include <erslib/dbio/clauses/column.hpp>
#include <erslib/dbio/clauses/condition.hpp>
#include <erslib/dbio/clauses/in.hpp>
#include <erslib/dbio/clauses/null.hpp>
#include <erslib/dbio/clauses/order.hpp>
#include <erslib/dbio/clauses/raw.hpp>
