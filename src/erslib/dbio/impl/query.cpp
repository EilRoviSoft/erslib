#include "erslib/dbio/impl/query.hpp"

// std
#include <algorithm>

// ers
#include <erslib/dbio/eval.hpp>


// Modifiers

void dbio::impl::Query::add(Clause clause) {
    if (clause->slot()->arity == Arity::Single) {
        std::erase_if(_clauses, [&](const Clause& it) {
            return same_slot(it->slot(), clause->slot());
        });
    }

    _clauses.emplace_back(std::move(clause));
}

void dbio::impl::Query::add(std::vector<Clause> clauses) {
    for (auto& it : clauses)
        add(std::move(it));
}


ers::Status dbio::impl::Query::build(Context& ctx) const {
    if (auto s = _validate(); !s)
        return s;

    for (const auto& binding : _layout.bindings())
        if (auto s = _render_slot(ctx, binding); !s)
            return s;

    return ers::ok;
}

ers::Status dbio::impl::Query::_render_slot(Context& ctx, const SlotBinding& binding) const {
    if (binding.renderer)
        return _render_custom(ctx, binding);

    bool empty = true;

    for (const auto& it : _clauses) {
        if (!same_slot(it->slot(), binding.slot))
            continue;

        ctx.query += empty ? binding.prefix : binding.separator;
        empty = false;

        if (auto s = it->render(ctx); !s)
            return s;
    }

    if (empty) {
        if (binding.fallback.empty())
            return ers::ok;

        ctx.query += binding.prefix;
        ctx.query += binding.fallback;
    }

    ctx.query += binding.suffix;

    return ers::ok;
}

ers::Status dbio::impl::Query::_render_custom(Context& ctx, const SlotBinding& binding) const {
    std::vector<const Clause*> group;

    for (const auto& it : _clauses) {
        if (same_slot(it->slot(), binding.slot))
            group.push_back(&it);
    }

    if (group.empty()) {
        if (binding.fallback.empty())
            return ers::ok;

        ctx.query += binding.prefix;
        ctx.query += binding.fallback;
        ctx.query += binding.suffix;

        return ers::ok;
    }

    return binding.renderer(ctx, group);
}

ers::Status dbio::impl::Query::_validate() const {
    for (const auto& it : _clauses) {
        if (!_layout.contains(it->slot())) {
            return ers::make_error("Slot '{}' is not part of this statement",
                it->slot()->name);
        }
    }

    return ers::ok;
}


ers::Result<std::string> dbio::impl::Query::to_sql() const {
    Context ctx;

    if (auto s = build(ctx); !s)
        return s.error();

    return std::move(ctx.query);
}


// Executors

ers::Result<pqxx::result> dbio::impl::Query::exec(pqxx::dbtransaction& tx) const ERS_DBIO_TRY_EVAL {
    Context ctx(tx);

    if (auto s = build(ctx); !s)
        return s.error();

    return tx.exec(ctx.query, ctx.params);
}
ERS_DBIO_CATCH_EVAL_ERRORS

ers::Status dbio::impl::Query::exec_and_discard(pqxx::dbtransaction& tx) const {
    auto r = exec(tx);
    if (!r)
        return r.error();

    return ers::ok;
}


// Operators

dbio::impl::Query& dbio::impl::operator|(Query& lhs, Clause rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}

dbio::impl::Query&& dbio::impl::operator|(Query&& lhs, Clause rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

dbio::impl::Query& dbio::impl::operator|=(Query& lhs, Clause rhs) {
    return lhs | std::move(rhs);
}

dbio::impl::Query& dbio::impl::operator|(Query& lhs, std::vector<Clause> rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}

dbio::impl::Query&& dbio::impl::operator|(Query&& lhs, std::vector<Clause> rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

dbio::impl::Query& dbio::impl::operator|=(Query& lhs, std::vector<Clause> rhs) {
    return lhs | std::move(rhs);
}
