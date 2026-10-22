#pragma once

// std
#include <string>
#include <string_view>
#include <utility>

// ers
#include "common.inl"


namespace flux::impl::nodes {
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
}
