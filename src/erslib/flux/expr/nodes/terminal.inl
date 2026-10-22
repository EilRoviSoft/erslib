#pragma once

// std
#include <string>
#include <string_view>
#include <utility>

// ers
#include "common.inl"


namespace flux::impl::nodes {
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
            if (!is_identifier(_name))
                return ers::make_error("Invalid identifier '{}' in an expression.", _name);

            ctx.query += _name;
            return ers::ok;
        }


    private:
        std::string _name;
    };
}
