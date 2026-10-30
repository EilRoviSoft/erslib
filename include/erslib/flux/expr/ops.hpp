#pragma once

// std
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/flux/expr/expr.hpp>


// Implementation --------------------------------------------------------------

// Operators and functions: the name is a template argument, so it never comes from a runtime string.

namespace flux::impl {
    template<ers::fixed_string Op, EPrecedence Precedence = EPrecedence::Other>
    struct binary_op_t {
        static_assert(!Op.to_sv().empty(), "binary_op_t<> needs an operator.");

        Expr operator()(Expr lhs, Expr rhs) const {
            return make_binary(Op.to_sv(), std::move(lhs), std::move(rhs), Precedence);
        }
    };

    template<ers::fixed_string Name>
    struct fn_t {
        static_assert(!Name.to_sv().empty(), "fn_t<> needs a name.");

        template<typename... Args>
        Expr operator()(Args&&... args) const {
            std::vector<Expr> items;
            items.reserve(sizeof...(Args));
            (items.push_back(Expr(std::forward<Args>(args))), ...);

            return make_fn(Name.to_sv(), std::move(items));
        }
    };
}


// Raw SQL: '{}' takes the next argument, a value becomes a parameter.

namespace flux::impl::clauses {
    template<typename... Args>
    Expr sql(sql_text pattern, Args&&... args) {
        std::vector<Expr> items;
        items.reserve(sizeof...(Args));
        (items.push_back(Expr(std::forward<Args>(args))), ...);

        return make_raw(pattern, std::move(items));
    }
}
