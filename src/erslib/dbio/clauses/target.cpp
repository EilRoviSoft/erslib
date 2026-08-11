#include "erslib/dbio/clauses/target.hpp"

// std
#include <ranges>

// ers
#include <erslib/dbio/clauses/raw.hpp>


// StClause

dbio::StClause::StClause(Section section, std::string name) :
    IClause(section),
    _name(std::move(name)) {
}
ers::Status dbio::StClause::render(build_context_t& ctx) const {
    if (!internal::is_identifier(_name) && _name != "*")
        return ers::make_error("dbio::Query: invalid column name '{}'", _name);

    ctx.query += _name;
    return ers::ok;
}
dbio::ClausePtr dbio::StClause::clone() const {
    return std::make_unique<StClause>(section(), _name);
}


// MtClause

dbio::MtClause::MtClause(Section section, std::vector<std::string> names, bool wrap) :
    IClause(section),
    _names(std::move(names)),
    _wrap(wrap) {
}
ers::Status dbio::MtClause::render(build_context_t& ctx) const {
    if (_wrap)
        ctx.query += "(";

    for (const auto& [i, it] : _names | std::views::enumerate) {
        if (!internal::is_identifier(it))
            return ers::make_error("dbio::Query: invalid column name '{}'", it);

        if (i > 0)
            ctx.query += ", ";
        ctx.query += it;
    }

    if (_wrap)
        ctx.query += ")";

    return ers::ok;
}
dbio::ClausePtr dbio::MtClause::clone() const {
    return std::make_unique<MtClause>(section(), _names, _wrap);
}


// Shortcuts

dbio::ClausePtr dbio::clauses::column(std::string name) {
    return std::make_unique<StClause>(section::column, std::move(name));
}

dbio::ClausePtr dbio::clauses::all_columns() {
    return std::make_unique<StClause>(section::column, "*");
}
