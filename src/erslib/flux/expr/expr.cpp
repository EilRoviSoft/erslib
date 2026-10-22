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

flux::impl::Expr flux::impl::make_postfix(Expr operand, sql_text suffix, EPrecedence precedence) {
    return make_expr<nodes::PostfixNode>(std::move(operand), suffix.text, precedence);
}

flux::impl::Expr flux::impl::make_between(Expr operand, Expr low, Expr high) {
    return make_expr<nodes::BetweenNode>(std::move(operand), std::move(low), std::move(high));
}

flux::impl::Expr flux::impl::make_in(Expr operand, std::vector<Expr> items) {
    return make_expr<nodes::InNode>(std::move(operand), std::move(items));
}

flux::impl::Expr flux::impl::make_in(Expr operand, QueryBuilder subquery) {
    return make_expr<nodes::InQueryNode>(std::move(operand), std::move(subquery));
}

flux::impl::Expr flux::impl::make_any(Expr operand, Expr array) {
    return make_expr<nodes::AnyNode>(std::move(operand), std::move(array));
}

flux::impl::Expr flux::impl::make_cast(Expr operand, std::string_view type) {
    return make_expr<nodes::CastNode>(std::move(operand), std::string(type));
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
