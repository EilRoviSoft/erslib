#include "erslib/flux/query/clauses/where.hpp"


flux::impl::ConditionClause::ConditionClause(SlotRef slot, Expr condition) :
    IClause(slot),
    _condition(std::move(condition)) {
}

ers::Status flux::impl::ConditionClause::render(Context& ctx) const {
    return _condition.render(ctx, EPrecedence::And);
}
