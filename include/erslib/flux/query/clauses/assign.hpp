#pragma once

// std
#include <cstddef>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT AssignClause : public IClause {
    public:
        // Member functions

        AssignClause(std::string column, Expr value);


        // Executors

        ers::Status render(Context& ctx) const override;

        ers::Status render_column(Context& ctx) const;
        ers::Status render_value(Context& ctx) const;


    private:
        std::string _column;
        Expr _value;
    };


    namespace clauses {
        // A value is bound, an Expr is rendered: assign("stock", col("stock") - qty).
        template<typename T>
        Clause assign(std::string column, T&& value) {
            return make_clause<AssignClause>(std::move(column), Expr(std::forward<T>(value)));
        }

        ERSLIB_EXPORT Clause assign_null(std::string column);


        // Pairs of a column and a value: set("name", "bob", "stock", col("stock") - 1).
        template<typename... Args>
            requires (sizeof...(Args) >= 2 && sizeof...(Args) % 2 == 0)
        std::vector<Clause> set(Args&&... args) {
            std::vector<Clause> out;
            out.reserve(sizeof...(Args) / 2);

            auto pairs = std::forward_as_tuple(std::forward<Args>(args)...);

            [&]<size_t... I>(std::index_sequence<I...>) {
                (out.push_back(assign(
                    std::string(std::get<2 * I>(pairs)),
                    std::get<2 * I + 1>(std::move(pairs))
                )), ...);
            }(std::make_index_sequence<sizeof...(Args) / 2> {});

            return out;
        }
    }
}
