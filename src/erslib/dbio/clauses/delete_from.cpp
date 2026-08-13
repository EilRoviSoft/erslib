#include "erslib/dbio/clauses/delete_from.hpp"

// ers
#include <erslib/dbio/clauses/target.hpp>


dbio::impl::Query dbio::impl::clauses::delete_from(std::string table) {
    Query q;

    q |= std::make_unique<StClause>(section::delete_from, std::move(table));
    
    return q;
}
