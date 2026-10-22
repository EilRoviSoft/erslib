#pragma once

// std
#include <utility>
#include <vector>

// ers
#include <erslib/flux/query/clauses/column.hpp>
#include <erslib/flux/query/slots/group.hpp>


namespace flux::impl::clauses {
    template<typename... Args>
        requires (std::constructible_from<ColumnExpr, Args> && ...)
    std::vector<Clause> group_by(Args&&... args) {
        return make_columns(&slots::group, std::forward<Args>(args)...);
    }

    template<ColumnNames R>
    std::vector<Clause> group_by(R&& names) {
        return make_columns(&slots::group, std::forward<R>(names));
    }
}
