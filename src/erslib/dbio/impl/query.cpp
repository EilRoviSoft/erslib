#include "erslib/dbio/impl/query.hpp"

// ers
#include <erslib/dbio/eval.hpp>


// Query

dbio::Query::Query(std::string table) :
    _table(std::move(table)) {
}

dbio::Query::Query(const Query& other) {
    _copy_from(other);
}
dbio::Query& dbio::Query::operator=(const Query& other) {
    _copy_from(other);
    return *this;
}


void dbio::Query::set_table(std::string table) {
    _table = std::move(table);
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
    if (!internal::is_identifier(_table))
        return ers::make_error("dbio::Query: invalid table name '{}'", _table);

    ctx.query += "SELECT ";

    if (_has(Section::Column)) {
        if (auto s = _render(ctx, Section::Column); !s)
            return s;
    } else
        ctx.query += '*';

    ctx.query += "\nFROM ";
    ctx.query += _table;

    static constexpr std::array tail = { Section::Where, Section::OrderBy, Section::Limit, Section::Offset };

    for (const Section section : tail) {
        if (auto s = _render(ctx, section); !s)
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


bool dbio::Query::_has(Section section) const {
    return std::ranges::any_of(_clauses, [&](const ClausePtr& it) {
        return it->section() == section;
    });
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

    return ers::ok;
}

void dbio::Query::_copy_from(const Query& other) {
    _table = other._table;

    _clauses.clear();
    _clauses.reserve(other._clauses.size());

    for (const auto& it : other._clauses)
        _clauses.emplace_back(it->clone());
}


dbio::Query dbio::from(std::string table) {
    return Query(std::move(table));
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
