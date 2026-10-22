#pragma once

// std
#include <utility>

// ers
#include <erslib/flux/query/query_builder.hpp>
#include "common.inl"


namespace flux::impl::nodes {
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
}
