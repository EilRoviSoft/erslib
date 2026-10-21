#pragma once

// std
#include <concepts>
#include <string>
#include <utility>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT JoinClause : public IClause {
    public:
        // Member functions

        JoinClause(std::string table, std::string alias, Expr on);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _table;
        std::string _alias;
        Expr _on;
    };


    namespace clauses {
        // Expr only: 'join("t", "alias")' must not silently bind a string.
        template<std::same_as<Expr> T>
        Clause join(std::string table, std::string alias, T on) {
            return make_clause<JoinClause>(std::move(table), std::move(alias), std::move(on));
        }

        template<std::same_as<Expr> T>
        Clause join(std::string table, T on) {
            return make_clause<JoinClause>(std::move(table), std::string(), std::move(on));
        }
    }
}
