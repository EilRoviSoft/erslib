#include "erslib/dbio/clauses/select_from.hpp"

// ers
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/clauses/target.hpp>


dbio::impl::Query dbio::impl::clauses::select_from(std::string table) {
    Query q;

    q |= raw(section::select, "SELECT ");
    q |= std::make_unique<StClause>(section::source, std::move(table));

    return q;
}
