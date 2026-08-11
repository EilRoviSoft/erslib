#include "erslib/dbio/clauses/insert_into.hpp"

#include "erslib/dbio/clauses/target.hpp"


dbio::impl::Query dbio::impl::clauses::insert_into(std::string table) {
    Query q;

    q |= std::make_unique<StClause>(section::insert_into, std::move(table));

    return q;
}
