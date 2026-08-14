#include "erslib/dbio/clauses/raw.hpp"


// RawClause

dbio::impl::RawClause::RawClause(SlotRef slot, std::string sql, std::vector<binder_t> binders) :
    IClause(slot),
    _sql(std::move(sql)),
    _binders(std::move(binders)) {
}

ers::Status dbio::impl::RawClause::render(Context& ctx) const {
    size_t next = 0;

    for (const char it : _sql) {
        if (it != '?') {
            ctx.query += it;
            continue;
        }

        if (next == _binders.size()) {
            return ers::make_error("Slot '{}' has more '?' markers than values ({}).",
                slot()->name, _binders.size());
        }

        const auto& binder = _binders[next];
        ctx.query += binder ? ctx.bind(binder) : ctx.bind_null();

        next++;
    }

    if (next != _binders.size()) {
        return ers::make_error("Slot '{}' has {} values for only {} '?' markers.",
            slot()->name, _binders.size(), next);
    }

    return ers::ok;
}

dbio::impl::ClausePtr dbio::impl::RawClause::clone() const {
    return std::make_unique<RawClause>(slot(), _sql, _binders);
}
