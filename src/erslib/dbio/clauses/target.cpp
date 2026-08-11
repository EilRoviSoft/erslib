#include "erslib/dbio/clauses/target.hpp"

// std
#include <ranges>

// ers
#include <erslib/dbio/clauses/raw.hpp>


// StClause

dbio::impl::StClause::StClause(Section section, std::string name) :
    IClause(section),
    _name(std::move(name)) {
}
ers::Status dbio::impl::StClause::render(build_context_t& ctx) const {
    if (!is_identifier(_name) && _name != "*")
        return ers::make_error("dbio::Query: invalid column name '{}'", _name);

    ctx.query += _name;
    return ers::ok;
}
dbio::impl::ClausePtr dbio::impl::StClause::clone() const {
    return std::make_unique<StClause>(section(), _name);
}


// MtClause

dbio::impl::MtClause::MtClause(Section section, std::vector<std::string> names, bool wrap) :
    IClause(section),
    _names(std::move(names)),
    _wrap(wrap) {
}
ers::Status dbio::impl::MtClause::render(build_context_t& ctx) const {
    if (_wrap)
        ctx.query += "(";

    for (const auto& [i, it] : _names | std::views::enumerate) {
        if (!is_identifier(it))
            return ers::make_error("dbio::Query: invalid column name '{}'", it);

        if (i > 0)
            ctx.query += ", ";
        ctx.query += it;
    }

    if (_wrap)
        ctx.query += ")";

    return ers::ok;
}
dbio::impl::ClausePtr dbio::impl::MtClause::clone() const {
    return std::make_unique<MtClause>(section(), _names, _wrap);
}


// Shortcuts

dbio::impl::ClausePtr dbio::impl::clauses::column(std::string name) {
    return std::make_unique<StClause>(section::column, std::move(name));
}

dbio::impl::ClausePtr dbio::impl::clauses::all_columns() {
    return std::make_unique<StClause>(section::column, "*");
}
