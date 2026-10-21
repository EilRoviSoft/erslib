#include "erslib/flux/expr/expr.hpp"

// ers
#include <erslib/flux/query/query_builder.hpp>
#include <erslib/flux/sql/identifier.hpp>


// Details ---------------------------------------------------------------------

namespace {
    using namespace flux::impl;


    constexpr EPrecedence next(EPrecedence precedence) noexcept {
        return precedence == EPrecedence::Primary
            ? precedence
            : static_cast<EPrecedence>(static_cast<u8>(precedence) + 1);
    }

    bool is_type_name(std::string_view type) {
        while (type.ends_with("[]"))
            type.remove_suffix(2);

        return flux::impl::is_identifier(type);
    }

    ers::Status render_list(Context& ctx, const std::vector<Expr>& items) {
        bool first = true;

        for (const auto& it : items) {
            if (!first)
                ctx.query += ", ";
            first = false;

            ERS_TRY(it.render(ctx));
        }

        return ers::ok;
    }
}


// Nodes

namespace {
    class ValueNode final : public IExprNode {
    public:
        explicit ValueNode(binder_t binder) :
            _binder(std::move(binder)) {
        }


        ers::Status render(Context& ctx) const override {
            ctx.query += ctx.bind(_binder);
            return ers::ok;
        }


    private:
        binder_t _binder;
    };


    class ParamNode final : public IExprNode {
    public:
        explicit ParamNode(binder_t binder) :
            _binder(std::move(binder)) {
        }


        ers::Status render(Context& ctx) const override {
            ctx.query += ctx.bind_once(this, _binder);
            return ers::ok;
        }


    private:
        binder_t _binder;
    };


    class ConstNode final : public IExprNode {
    public:
        explicit ConstNode(std::string_view text) :
            _text(text) {
        }


        ers::Status render(Context& ctx) const override {
            ctx.query += _text;
            return ers::ok;
        }


    private:
        std::string_view _text;
    };


    class ColumnNode final : public IExprNode {
    public:
        explicit ColumnNode(std::string name) :
            _name(std::move(name)) {
        }


        ers::Status render(Context& ctx) const override {
            if (!flux::impl::is_identifier(_name))
                return ers::make_error("Invalid identifier '{}' in an expression.", _name);

            ctx.query += _name;
            return ers::ok;
        }


    private:
        std::string _name;
    };


    class BinaryNode final : public IExprNode {
    public:
        BinaryNode(std::string op, Expr lhs, Expr rhs, EPrecedence precedence, bool left_associative) :
            _op(std::move(op)),
            _lhs(std::move(lhs)),
            _rhs(std::move(rhs)),
            _precedence(precedence),
            _left_associative(left_associative) {
        }


        EPrecedence precedence() const noexcept override { return _precedence; }


        ers::Status render(Context& ctx) const override {
            ERS_TRY(_lhs.render(ctx, _left_associative ? _precedence : next(_precedence)));

            ctx.query += ' ';
            ctx.query += _op;
            ctx.query += ' ';

            return _rhs.render(ctx, next(_precedence));
        }


    private:
        std::string _op;
        Expr _lhs;
        Expr _rhs;
        EPrecedence _precedence;
        bool _left_associative;
    };


    class LogicalNode final : public IExprNode {
    public:
        LogicalNode(bool conjunction, std::vector<Expr> items) :
            _conjunction(conjunction),
            _items(std::move(items)) {
        }


        EPrecedence precedence() const noexcept override {
            return _conjunction ? EPrecedence::And : EPrecedence::Or;
        }


        bool conjunction() const noexcept { return _conjunction; }
        const std::vector<Expr>& items() const noexcept { return _items; }


        ers::Status render(Context& ctx) const override {
            bool first = true;

            for (const auto& it : _items) {
                if (!first)
                    ctx.query += _conjunction ? " AND " : " OR ";
                first = false;

                ERS_TRY(it.render(ctx, precedence()));
            }

            return ers::ok;
        }


    private:
        bool _conjunction;
        std::vector<Expr> _items;
    };


    class NotNode final : public IExprNode {
    public:
        explicit NotNode(Expr what) :
            _what(std::move(what)) {
        }


        EPrecedence precedence() const noexcept override { return EPrecedence::Not; }


        ers::Status render(Context& ctx) const override {
            ctx.query += "NOT ";
            return _what.render(ctx, EPrecedence::Not);
        }


    private:
        Expr _what;
    };


    class PostfixNode final : public IExprNode {
    public:
        PostfixNode(Expr operand, std::string_view suffix, EPrecedence precedence) :
            _operand(std::move(operand)),
            _suffix(suffix),
            _precedence(precedence) {
        }


        EPrecedence precedence() const noexcept override { return _precedence; }


        ers::Status render(Context& ctx) const override {
            ERS_TRY(_operand.render(ctx, next(_precedence)));

            ctx.query += _suffix;
            return ers::ok;
        }


    private:
        Expr _operand;
        std::string_view _suffix;
        EPrecedence _precedence;
    };


    class CastNode final : public IExprNode {
    public:
        CastNode(Expr operand, std::string type) :
            _operand(std::move(operand)),
            _type(std::move(type)) {
        }


        EPrecedence precedence() const noexcept override { return EPrecedence::Cast; }


        ers::Status render(Context& ctx) const override {
            if (!is_type_name(_type))
                return ers::make_error("Invalid type name '{}' in a cast.", _type);

            ERS_TRY(_operand.render(ctx, EPrecedence::Cast));

            ctx.query += "::";
            ctx.query += _type;
            return ers::ok;
        }


    private:
        Expr _operand;
        std::string _type;
    };


    class BetweenNode final : public IExprNode {
    public:
        BetweenNode(Expr operand, Expr low, Expr high) :
            _operand(std::move(operand)),
            _low(std::move(low)),
            _high(std::move(high)) {
        }


        EPrecedence precedence() const noexcept override { return EPrecedence::Like; }


        ers::Status render(Context& ctx) const override {
            ERS_TRY(_operand.render(ctx, EPrecedence::Other));

            ctx.query += " BETWEEN ";
            ERS_TRY(_low.render(ctx, EPrecedence::Other));

            ctx.query += " AND ";
            return _high.render(ctx, EPrecedence::Other);
        }


    private:
        Expr _operand;
        Expr _low;
        Expr _high;
    };


    class InNode final : public IExprNode {
    public:
        InNode(Expr operand, std::vector<Expr> items) :
            _operand(std::move(operand)),
            _items(std::move(items)) {
        }


        EPrecedence precedence() const noexcept override {
            return _items.empty() ? EPrecedence::Primary : EPrecedence::Like;
        }


        ers::Status render(Context& ctx) const override {
            if (_items.empty()) {
                ctx.query += "FALSE";
                return ers::ok;
            }

            ERS_TRY(_operand.render(ctx, EPrecedence::Other));

            ctx.query += " IN (";
            ERS_TRY(render_list(ctx, _items));

            ctx.query += ')';
            return ers::ok;
        }


    private:
        Expr _operand;
        std::vector<Expr> _items;
    };


    class InQueryNode final : public IExprNode {
    public:
        InQueryNode(Expr operand, QueryBuilder query) :
            _operand(std::move(operand)),
            _query(std::move(query)) {
        }


        EPrecedence precedence() const noexcept override { return EPrecedence::Like; }


        ers::Status render(Context& ctx) const override {
            ERS_TRY(_operand.render(ctx, EPrecedence::Other));

            return append_subquery(ctx, _query, " IN (");
        }


    private:
        Expr _operand;
        QueryBuilder _query;
    };


    class AnyNode final : public IExprNode {
    public:
        AnyNode(Expr operand, Expr array) :
            _operand(std::move(operand)),
            _array(std::move(array)) {
        }


        EPrecedence precedence() const noexcept override { return EPrecedence::Comparison; }


        ers::Status render(Context& ctx) const override {
            ERS_TRY(_operand.render(ctx, EPrecedence::Like));

            ctx.query += " = ANY(";
            ERS_TRY(_array.render(ctx));

            ctx.query += ')';
            return ers::ok;
        }


    private:
        Expr _operand;
        Expr _array;
    };


    class FnNode final : public IExprNode {
    public:
        FnNode(std::string_view name, std::vector<Expr> args) :
            _name(name),
            _args(std::move(args)) {
        }


        ers::Status render(Context& ctx) const override {
            if (!flux::impl::is_identifier(_name))
                return ers::make_error("Invalid function name '{}'.", _name);

            ctx.query += _name;
            ctx.query += '(';
            ERS_TRY(render_list(ctx, _args));

            ctx.query += ')';
            return ers::ok;
        }


    private:
        std::string_view _name;
        std::vector<Expr> _args;
    };


    class RowNode final : public IExprNode {
    public:
        explicit RowNode(std::vector<Expr> items) :
            _items(std::move(items)) {
        }


        ers::Status render(Context& ctx) const override {
            ctx.query += '(';
            ERS_TRY(render_list(ctx, _items));

            ctx.query += ')';
            return ers::ok;
        }


    private:
        std::vector<Expr> _items;
    };


    class RawNode final : public IExprNode {
    public:
        RawNode(std::string_view pattern, std::vector<Expr> args) :
            _pattern(pattern),
            _args(std::move(args)) {
        }


        EPrecedence precedence() const noexcept override { return EPrecedence::Or; }


        ers::Status render(Context& ctx) const override {
            std::string_view rest = _pattern;
            size_t used = 0;

            for (auto at = rest.find("{}"); at != std::string_view::npos; at = rest.find("{}")) {
                if (used == _args.size())
                    return ers::make_error("Raw SQL '{}' has more '{{}}' than arguments.", _pattern);

                ctx.query += rest.substr(0, at);
                ERS_TRY(_args[used++].render(ctx, EPrecedence::Primary));

                rest.remove_prefix(at + 2);
            }

            if (used != _args.size())
                return ers::make_error("Raw SQL '{}' has fewer '{{}}' than arguments.", _pattern);

            ctx.query += rest;
            return ers::ok;
        }


    private:
        std::string_view _pattern;
        std::vector<Expr> _args;
    };


    class SubqueryNode final : public IExprNode {
    public:
        SubqueryNode(QueryBuilder query, bool exists) :
            _query(std::move(query)),
            _exists(exists) {
        }


        ers::Status render(Context& ctx) const override {
            return append_subquery(ctx, _query, _exists ? "EXISTS (" : "(");
        }


    private:
        QueryBuilder _query;
        bool _exists;
    };


    template<typename T, typename... Args>
    Expr make_node(Args&&... args) {
        return Expr(std::make_shared<const T>(std::forward<Args>(args)...));
    }
}


// Implementation --------------------------------------------------------------

namespace flux::impl {
    ExprNodePtr make_value_node(binder_t binder) {
        return std::make_shared<const ValueNode>(std::move(binder));
    }

    ExprNodePtr make_param_node(binder_t binder) {
        return std::make_shared<const ParamNode>(std::move(binder));
    }
}


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
    return make_node<PostfixNode>(*this, " IS NULL", EPrecedence::Is);
}

flux::impl::Expr flux::impl::Expr::is_not_null() const {
    return make_node<PostfixNode>(*this, " IS NOT NULL", EPrecedence::Is);
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
    return make_node<BetweenNode>(*this, std::move(low), std::move(high));
}

flux::impl::Expr flux::impl::Expr::contains(Expr other) const {
    return make_binary("@>", *this, std::move(other), EPrecedence::Other);
}

flux::impl::Expr flux::impl::Expr::in(std::initializer_list<Expr> values) const {
    return _in(std::vector<Expr>(values));
}

flux::impl::Expr flux::impl::Expr::in(QueryBuilder subquery) const {
    return make_node<InQueryNode>(*this, std::move(subquery));
}

flux::impl::Expr flux::impl::Expr::any(Expr array) const {
    return make_node<AnyNode>(*this, std::move(array));
}

flux::impl::Expr flux::impl::Expr::cast(std::string_view type) const {
    return make_node<CastNode>(*this, std::string(type));
}

flux::impl::Expr flux::impl::Expr::_in(std::vector<Expr> items) const {
    return make_node<InNode>(*this, std::move(items));
}


// Node factories

flux::impl::Expr flux::impl::make_binary(std::string_view op, Expr lhs, Expr rhs, EPrecedence precedence, bool left_associative) {
    return make_node<BinaryNode>(std::string(op), std::move(lhs), std::move(rhs), precedence, left_associative);
}

flux::impl::Expr flux::impl::make_fn(std::string_view name, std::vector<Expr> args) {
    return make_node<FnNode>(name, std::move(args));
}

flux::impl::Expr flux::impl::make_const(sql_text text) {
    return make_node<ConstNode>(text.text);
}

flux::impl::Expr flux::impl::make_raw(sql_text pattern, std::vector<Expr> args) {
    return make_node<RawNode>(pattern.text, std::move(args));
}

flux::impl::Expr flux::impl::make_row(std::vector<Expr> items) {
    return make_node<RowNode>(std::move(items));
}

flux::impl::Expr flux::impl::make_logical(bool conjunction, Expr lhs, Expr rhs) {
    std::vector<Expr> items;

    for (auto* side : { &lhs, &rhs }) {
        const auto* logical = dynamic_cast<const LogicalNode*>(&side->node());

        if (logical && logical->conjunction() == conjunction)
            items.append_range(logical->items());
        else
            items.emplace_back(std::move(*side));
    }

    return make_node<LogicalNode>(conjunction, std::move(items));
}

flux::impl::Expr flux::impl::make_not(Expr what) {
    return make_node<NotNode>(std::move(what));
}


// Shortcuts

flux::impl::Expr flux::impl::clauses::col(std::string name) {
    return make_node<ColumnNode>(std::move(name));
}

flux::impl::Expr flux::impl::clauses::col(std::string_view table, std::string_view name) {
    return col(std::string(table) + '.' + std::string(name));
}

flux::impl::Expr flux::impl::clauses::null() {
    return make_const("NULL");
}

flux::impl::Expr flux::impl::clauses::exists(QueryBuilder subquery) {
    return make_node<SubqueryNode>(std::move(subquery), true);
}

flux::impl::Expr flux::impl::clauses::subquery_of(QueryBuilder subquery) {
    return make_node<SubqueryNode>(std::move(subquery), false);
}
