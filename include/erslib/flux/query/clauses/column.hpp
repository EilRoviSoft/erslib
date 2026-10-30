#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/query/clause.hpp>
#include <erslib/flux/query/slots/column.hpp>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    // An item of a list slot: the select list, RETURNING, GROUP BY.
    class ERSLIB_EXPORT ColumnClause : public IClause {
    public:
        // Member functions

        ColumnClause(SlotRef slot, Expr what, std::string alias = "");


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        Expr _what;
        std::string _alias;
    };


    template<typename T>
    concept ColumnItem = std::constructible_from<ColumnExpr, T> || std::same_as<std::remove_cvref_t<T>, Aliased>;

    template<typename R>
    concept ColumnNames = std::ranges::input_range<R>
        && std::constructible_from<std::string, std::ranges::range_reference_t<R>>;


    ERSLIB_EXPORT Clause make_column(SlotRef slot, ColumnExpr what);
    ERSLIB_EXPORT Clause make_column(SlotRef slot, Aliased what);

    template<ColumnItem... Args>
    std::vector<Clause> make_columns(SlotRef slot, Args&&... args) {
        std::vector<Clause> out;
        out.reserve(sizeof...(Args));
        (out.push_back(make_column(slot, std::forward<Args>(args))), ...);
        return out;
    }

    template<ColumnNames R>
    std::vector<Clause> make_columns(SlotRef slot, R&& names) {
        std::vector<Clause> out;

        if constexpr (std::ranges::sized_range<R>)
            out.reserve(std::ranges::size(names));

        for (auto&& it : names)
            out.push_back(make_column(slot, std::string(std::forward<decltype(it)>(it))));

        return out;
    }


    namespace clauses {
        ERSLIB_EXPORT Clause column(ColumnExpr what);
        ERSLIB_EXPORT Clause column(Aliased what);

        template<ColumnItem... Args>
        std::vector<Clause> columns(Args&&... args) {
            return make_columns(&slots::column, std::forward<Args>(args)...);
        }

        template<ColumnNames R>
        std::vector<Clause> columns(R&& names) {
            return make_columns(&slots::column, std::forward<R>(names));
        }
    }
}
