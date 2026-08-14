#include "erslib/dbio/clauses/general.hpp"


// Internal

namespace {
    constexpr dbio::impl::SlotRef select_layout[] = {
        &dbio::impl::column_slot,
        &dbio::impl::from_slot,
    };
}


dbio::impl::Query dbio::impl::clauses::select_from(std::string table) {
    Query q(select_layout);

    q |= from(std::move(table));

    return q;
}

dbio::impl::ClausePtr dbio::impl::clauses::column(std::string name) {
    return identifier(&column_slot, std::move(name));
}

dbio::impl::ClausePtr dbio::impl::clauses::from(std::string name) {
    return identifier(&from_slot, std::move(name));
}
