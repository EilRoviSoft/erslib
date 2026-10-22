#pragma once

// std
#include <string_view>
#include <utility>
#include <vector>

// ers
#include "common.inl"


namespace flux::impl::nodes {
    class FnNode final : public IExprNode {
    public:
        FnNode(std::string_view name, std::vector<Expr> args) :
            _name(name),
            _args(std::move(args)) {
        }


        ers::Status render(Context& ctx) const override {
            if (!is_identifier(_name))
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
}
