#pragma once

// Goal is to have something like this:
// auto users = (select_from("users")
//     | where("guild_id", guild_id)
//     | where("score", Op::Ge, 100)
//     | order_by("created_at", Order::Desc)
//     | with_limit(20)
// ).exec_as<User>(tx);
//
// auto status = (insert_into("images")
//     | targets("url", "path")
//     | values(url_a, path_a)
//     | values(url_b, path_b)
// ).exec(tx);


// Main implementation

#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/query.hpp>

// Clauses

#include <erslib/dbio/clauses/bound.hpp>
#include <erslib/dbio/clauses/condition.hpp>
#include <erslib/dbio/clauses/in.hpp>
#include <erslib/dbio/clauses/insert_into.hpp>
#include <erslib/dbio/clauses/null.hpp>
#include <erslib/dbio/clauses/order.hpp>
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/clauses/select_from.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/values.hpp>
