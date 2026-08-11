#include "erslib/dbio/impl/query.hpp"

// std
#include <algorithm>

// ers
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/eval.hpp>


// Query

dbio::Query::Query(const Query& other) {
    _copy_from(other);
}
dbio::Query& dbio::Query::operator=(const Query& other) {
    _copy_from(other);
    return *this;
}


void dbio::Query::add(ClausePtr clause) {
    if (!clause)
        return;

    if (internal::is_singular(clause->section())) {
        std::erase_if(_clauses, [&](const ClausePtr& it) {
            return it->section() == clause->section();
        });
    }

    _clauses.emplace_back(std::move(clause));
}

ers::Status dbio::Query::build(build_context_t& ctx) const {
    std::vector<Section> sections;
    
    for (const auto& clause : _clauses) {
        if (std::ranges::find(sections, clause->section()) == sections.end())
            sections.push_back(clause->section());
    }


    std::ranges::sort(sections);

    for (const Section sec : sections) {
        if (auto s = _render(ctx, sec); !s)
            return s;
    }


    return ers::ok;
}

ers::Result<std::string> dbio::Query::to_sql() const {
    build_context_t ctx;

    if (auto s = build(ctx); !s)
        return s.error();

    return std::move(ctx.query);
}

ers::Result<pqxx::result> dbio::Query::exec(pqxx::dbtransaction& tx) const ERS_DBIO_TRY_EVAL {
    build_context_t ctx(tx);

    if (auto s = build(ctx); !s)
        return s.error();

    return tx.exec(ctx.query, ctx.params);
}
ERS_DBIO_CATCH_EVAL_ERRORS

ers::Status dbio::Query::exec_and_discard(pqxx::dbtransaction& tx) const {
    auto r = exec(tx);
    if (!r)
        return r.error();

    return ers::ok;
}


ers::Status dbio::Query::_render(build_context_t& ctx, Section section) const {
    auto format = internal::section_format(section);
    bool first = true;

    for (const auto& clause : _clauses) {
        if (clause->section() != section)
            continue;

        ctx.query += first ? format.prefix : format.separator;
        first = false;

        if (auto s = clause->render(ctx); !s)
            return s;
    }

    if (!first)
        ctx.query += format.suffix;

    return ers::ok;
}

void dbio::Query::_copy_from(const Query& other) {
    _clauses.clear();
    _clauses.reserve(other._clauses.size());

    for (const auto& it : other._clauses)
        _clauses.emplace_back(it->clone());
}


// Operators

dbio::Query& dbio::operator|(Query& lhs, ClausePtr rhs) {
    lhs.add(std::move(rhs));
    return lhs;
}

dbio::Query&& dbio::operator|(Query&& lhs, ClausePtr rhs) {
    lhs.add(std::move(rhs));
    return std::move(lhs);
}

dbio::Query& dbio::operator|=(Query& lhs, ClausePtr rhs) {
    return lhs | std::move(rhs);
}
