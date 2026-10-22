#include "erslib/flux/expr/expr.hpp"

// ers
#include "nodes/call.inl"
#include "nodes/logic.inl"
#include "nodes/operator.inl"
#include "nodes/predicate.inl"
#include "nodes/subquery.inl"
#include "nodes/terminal.inl"


// Implementation --------------------------------------------------------------

// Expr

flux::impl::Expr::Expr(ExprNodePtr node) :
    _node(std::move(node)) {
}


flux::impl::EPrecedence flux::impl::Expr::precedence() const noexcept {
    return _node->precedence();
}

ers::Status flux::impl::Expr::render(Context& ctx, EPrecedence parent) const {
    if (precedence() >= parent)
        return _node->render(ctx);

    ctx.query += '(';
    ERS_TRY(_node->render(ctx));

    ctx.query += ')';
    return ers::ok;
}


flux::impl::Expr flux::impl::Expr::is_null() const {
    return make_expr<nodes::PostfixNode>(*this, " IS NULL", EPrecedence::Is);
}

flux::impl::Expr flux::impl::Expr::is_not_null() const {
    return make_expr<nodes::PostfixNode>(*this, " IS NOT NULL", EPrecedence::Is);
}

flux::impl::Expr flux::impl::Expr::is_distinct_from(Expr other) const {
    return make_binary("IS DISTINCT FROM", *this, std::move(other), EPrecedence::Is);
}

flux::impl::Expr flux::impl::Expr::like(Expr pattern) const {
    return make_binary("LIKE", *this, std::move(pattern), EPrecedence::Like);
}

flux::impl::Expr flux::impl::Expr::ilike(Expr pattern) const {
    return make_binary("ILIKE", *this, std::move(pattern), EPrecedence::Like);
}

flux::impl::Expr flux::impl::Expr::between(Expr low, Expr high) const {
    return make_expr<nodes::BetweenNode>(*this, std::move(low), std::move(high));
}

flux::impl::Expr flux::impl::Expr::contains(Expr other) const {
    return make_binary("@>", *this, std::move(other), EPrecedence::Other);
}

flux::impl::Expr flux::impl::Expr::in(std::initializer_list<Expr> values) const {
    return _in(std::vector<Expr>(values));
}

flux::impl::Expr flux::impl::Expr::in(QueryBuilder subquery) const {
    return make_expr<nodes::InQueryNode>(*this, std::move(subquery));
}

flux::impl::Expr flux::impl::Expr::any(Expr array) const {
    return make_expr<nodes::AnyNode>(*this, std::move(array));
}

flux::impl::Expr flux::impl::Expr::cast(std::string_view type) const {
    return make_expr<nodes::CastNode>(*this, std::string(type));
}

flux::impl::Aliased flux::impl::Expr::as(std::string alias) const {
    return { *this, std::move(alias) };
}

flux::impl::Expr flux::impl::Expr::_in(std::vector<Expr> items) const {
    return make_expr<nodes::InNode>(*this, std::move(items));
}


// ColumnExpr

flux::impl::ColumnExpr::ColumnExpr(std::string name) :
    expr(clauses::col(std::move(name))) {
}

flux::impl::ColumnExpr::ColumnExpr(std::string_view name) :
    expr(clauses::col(std::string(name))) {
}

flux::impl::ColumnExpr::ColumnExpr(const char* name) :
    expr(clauses::col(name)) {
}


// Node factories

flux::impl::Expr flux::impl::make_value(binder_t binder) {
    return make_expr<nodes::ValueNode>(std::move(binder));
}

flux::impl::Expr flux::impl::make_param(binder_t binder) {
    return make_expr<nodes::ParamNode>(std::move(binder));
}

flux::impl::Expr flux::impl::make_const(sql_text text) {
    return make_expr<nodes::ConstNode>(text.text);
}

flux::impl::Expr flux::impl::make_binary(std::string_view op, Expr lhs, Expr rhs, EPrecedence precedence, bool left_associative) {
    return make_expr<nodes::BinaryNode>(std::string(op), std::move(lhs), std::move(rhs), precedence, left_associative);
}

flux::impl::Expr flux::impl::make_fn(std::string_view name, std::vector<Expr> args) {
    return make_expr<nodes::FnNode>(name, std::move(args));
}

flux::impl::Expr flux::impl::make_raw(sql_text pattern, std::vector<Expr> args) {
    return make_expr<nodes::RawNode>(pattern.text, std::move(args));
}

flux::impl::Expr flux::impl::make_row(std::vector<Expr> items) {
    return make_expr<nodes::RowNode>(std::move(items));
}

flux::impl::Expr flux::impl::make_logical(bool conjunction, Expr lhs, Expr rhs) {
    std::vector<Expr> items;

    for (auto* side : { &lhs, &rhs }) {
        const auto* logical = dynamic_cast<const nodes::LogicalNode*>(&side->node());

        if (logical && logical->conjunction() == conjunction)
            items.append_range(logical->items());
        else
            items.emplace_back(std::move(*side));
    }

    return make_expr<nodes::LogicalNode>(conjunction, std::move(items));
}

flux::impl::Expr flux::impl::make_not(Expr what) {
    return make_expr<nodes::NotNode>(std::move(what));
}


// Shortcuts

flux::impl::Expr flux::impl::clauses::col(std::string name) {
    return make_expr<nodes::ColumnNode>(std::move(name));
}

flux::impl::Expr flux::impl::clauses::col(std::string_view table, std::string_view name) {
    return col(std::string(table) + '.' + std::string(name));
}

flux::impl::Expr flux::impl::clauses::null() {
    return make_const("NULL");
}

flux::impl::Expr flux::impl::clauses::sql_bool(bool value) {
    return value ? make_const("TRUE") : make_const("FALSE");
}

flux::impl::Expr flux::impl::clauses::exists(QueryBuilder subquery) {
    return make_expr<nodes::SubqueryNode>(std::move(subquery), true);
}

flux::impl::Expr flux::impl::clauses::subquery(QueryBuilder query) {
    return make_expr<nodes::SubqueryNode>(std::move(query), false);
}
