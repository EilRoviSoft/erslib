#pragma once

// std
#include <ranges>
#include <string>
#include <utility>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/expr/ops.hpp>
#include <erslib/flux/traits.hpp>


// Details ---------------------------------------------------------------------

namespace flux::impl {
    template<std::ranges::input_range R>
    Expr is_empty_array(Expr array) {
        using value_t = owned_t<std::ranges::range_value_t<R>>;
        return fn_t<"cardinality"> {}(array.cast(std::string(sql_type<value_t>::name) + "[]")) == make_const("0");
    }
}


// Implementation --------------------------------------------------------------

namespace flux::impl::clauses {
    // cardinality($n::<type>[]) = 0
    template<std::ranges::input_range R>
    Expr empty_array(R&& values) {
        return is_empty_array<R>(param(std::forward<R>(values)));
    }

    // cardinality($n::<type>[]) = 0 OR <column> = ANY($n), the array bound once.
    template<std::ranges::input_range R>
    Expr any_or_empty(Expr column, R&& values) {
        const auto array = param(std::forward<R>(values));
        return is_empty_array<R>(array) || column.any(array);
    }
}
