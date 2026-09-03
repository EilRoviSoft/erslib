#include "erslib/conduit/impl/query_builder.hpp"

// std
#include <algorithm>

// ers
#include <erslib/core/exception.hpp>


// Modifiers

void conduit::impl::QueryBuilder::add(Clause clause) {
    if (clause->slot()->arity == EArity::Single) {
        std::erase_if(_clauses, [&](const Clause& it) {
            return same_slot(it->slot(), clause->slot());
        });
    }

    _clauses.emplace_back(std::move(clause));
}

void conduit::impl::QueryBuilder::add(std::vector<Clause> clauses) {
    for (auto& it : clauses)
        add(std::move(it));
}

void conduit::impl::QueryBuilder::add(WithCte with_cte) {
    _ctes.emplace_back(std::move(with_cte));
}

void conduit::impl::QueryBuilder::add(WithUnion with_union) {
    _unions.emplace_back(std::move(with_union));
}


ers::Status conduit::impl::QueryBuilder::build(Context& ctx) const {
    if (auto s = _validate(); !s)
        return s;

    if (!_ctes.empty()) {
        bool recursive = std::ranges::any_of(_ctes, [](const WithCte& c) { return c.recursive; });

        ctx.query += recursive ? "WITH RECURSIVE " : "WITH ";

        bool first = true;
        for (auto& cte : _ctes) {
            if (!first)
                ctx.query += ",\n";
            first = false;

            if (auto s = check_identifier(cte.name, nullptr); !s)
                return s;

            ctx.query += cte.name;
            ctx.query += " AS (\n";
            if (auto s = cte.definition.build(ctx); !s)
                return s;
            ctx.query += "\n)";
        }

        ctx.query += "\n";
    }

    for (const auto& binding : _layout.bindings()) {
        if (auto s = _render_slot(ctx, binding); !s)
            return s;
    }

    for (auto& u : _unions) {
        ctx.query += u.all ? "\nUNION ALL\n" : "\nUNION\n";
        if (auto s = u.other.build(ctx); !s)
            return s;
    }

    return ers::ok;
}

ers::Status conduit::impl::QueryBuilder::_render_slot(Context& ctx, const SlotBinding& binding) const {
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

ers::Status conduit::impl::QueryBuilder::_render_custom(Context& ctx, const SlotBinding& binding) const {
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

ers::Status conduit::impl::QueryBuilder::_validate() const {
    for (const auto& it : _clauses) {
        if (!_layout.contains(it->slot())) {
            return ers::make_error("Slot '{}' is not part of this statement",
                it->slot()->name);
        }
    }

    return ers::ok;
}


ers::Result<std::string> conduit::impl::QueryBuilder::to_sql() const {
    Context ctx;

    if (auto s = build(ctx); !s)
        return s.error();

    return std::move(ctx.query);
}


// Executors

conduit::impl::QueryResult conduit::impl::QueryBuilder::exec(pqxx::dbtransaction& tx) const {
    Context ctx(tx);

    if (auto s = build(ctx); !s)
        throw ers::make_runtime_error(s.error().to_string());

    return QueryResult(tx.exec(ctx.query, ctx.params));
}


// Operators

conduit::impl::QueryBuilder& conduit::impl::operator|(QueryBuilder& lhs, Clause rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
conduit::impl::QueryBuilder&& conduit::impl::operator|(QueryBuilder&& lhs, Clause rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}
conduit::impl::QueryBuilder& conduit::impl::operator|=(QueryBuilder& lhs, Clause rhs) {
    return lhs | std::move(rhs);
}

conduit::impl::QueryBuilder& conduit::impl::operator|(QueryBuilder& lhs, std::vector<Clause> rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
conduit::impl::QueryBuilder&& conduit::impl::operator|(QueryBuilder&& lhs, std::vector<Clause> rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}
conduit::impl::QueryBuilder& conduit::impl::operator|=(QueryBuilder& lhs, std::vector<Clause> rhs) {
    return lhs | std::move(rhs);
}


conduit::impl::QueryBuilder& conduit::impl::operator|(QueryBuilder& lhs, WithUnion rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
conduit::impl::QueryBuilder&& conduit::impl::operator|(QueryBuilder&& lhs, WithUnion rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}
conduit::impl::QueryBuilder& conduit::impl::operator|=(QueryBuilder& lhs, WithUnion rhs) {
    return lhs | std::move(rhs);
}

conduit::impl::QueryBuilder& conduit::impl::operator|(QueryBuilder& lhs, WithCte rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}
conduit::impl::QueryBuilder&& conduit::impl::operator|(QueryBuilder&& lhs, WithCte rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}
conduit::impl::QueryBuilder& conduit::impl::operator|=(QueryBuilder& lhs, WithCte rhs) {
    return lhs | std::move(rhs);
}


// Statement-level combinators

conduit::impl::WithCte conduit::impl::clauses::with_cte(std::string name, QueryBuilder definition, bool recursive) {
    return WithCte {
        .definition = std::move(definition),
        .name       = std::move(name),
        .recursive  = recursive
    };
}

conduit::impl::WithUnion conduit::impl::clauses::union_with(QueryBuilder other, bool all) {
    return WithUnion {
        .other = std::move(other),
        .all   = all
    };
}


// Subqueries

ers::Status conduit::impl::append_subquery(
    Context& ctx, const QueryBuilder& sub,
    std::string_view open, std::string_view close
) {
    ctx.query += open;

    if (auto s = sub.build(ctx); !s)
        return s;

    ctx.query += close;

    return ers::ok;
}
