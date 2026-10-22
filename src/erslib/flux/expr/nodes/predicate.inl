#pragma once

// std
#include <utility>
#include <vector>

// ers
#include "common.inl"


namespace flux::impl::nodes {
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
}
