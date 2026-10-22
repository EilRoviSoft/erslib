#pragma once

// std
#include <utility>
#include <vector>

// ers
#include "common.inl"


namespace flux::impl::nodes {
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
}
