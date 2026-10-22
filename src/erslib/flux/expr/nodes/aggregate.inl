#pragma once

// std
#include <string_view>
#include <utility>

// ers
#include <erslib/flux/expr/aggregate.hpp>
#include "common.inl"


namespace flux::impl::nodes {
    class AggregateNode final : public IExprNode {
    public:
        AggregateNode(std::string_view name, aggregate_t what) :
            _name(name),
            _what(std::move(what)) {
        }


        ers::Status render(Context& ctx) const override {
            if (!is_identifier(_name))
                return ers::make_error("Invalid aggregate name '{}'.", _name);

            if (_what.ordered_set && _what.within_group.empty())
                return ers::make_error("Ordered-set aggregate '{}' needs a WITHIN GROUP ordering.", _name);

            ctx.query += _name;
            ctx.query += '(';

            if (_what.distinct)
                ctx.query += "DISTINCT ";

            ERS_TRY(render_list(ctx, _what.args));

            if (!_what.order_by.empty()) {
                ctx.query += " ORDER BY ";
                ERS_TRY(render_list(ctx, _what.order_by));
            }

            ctx.query += ')';

            if (_what.ordered_set) {
                ctx.query += " WITHIN GROUP (ORDER BY ";
                ERS_TRY(render_list(ctx, _what.within_group));
                ctx.query += ')';
            }

            return ers::ok;
        }


    private:
        std::string_view _name;
        aggregate_t _what;
    };
}
