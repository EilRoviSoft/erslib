#pragma once

// std
#include <concepts>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/dbio/impl/binder.hpp>
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


// AggregateClause

namespace dbio::impl {
    // fn(col[, col...]) - the plain n-ary aggregates: count, min, max, sum, avg, array_agg,
    // bool_and, corr, the regr_* family, and friends.
    class ERSLIB_EXPORT AggregateClause : public IClause {
    public:
        // Member functions

        AggregateClause(std::string_view fn, std::vector<std::string> columns, bool distinct = false);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string_view _fn;
        std::vector<std::string> _columns;
        bool _distinct;
    };


    namespace clauses {
        // For anything without a shorthand: agg<"regr_slope">("t.y", "t.x").
        // Keep the name lowercase - count(*) is recognised by name.
        template<ers::fixed_string Fn, typename... Cols>
            requires (std::convertible_to<Cols, std::string> && ...)
        Clause agg(Cols&&... columns) {
            return make_clause<AggregateClause>(Fn.to_sv(),
                std::vector<std::string> { std::string(std::forward<Cols>(columns))... });
        }

        template<ers::fixed_string Fn>
        Clause agg_distinct(std::string column) {
            return make_clause<AggregateClause>(Fn.to_sv(),
                std::vector<std::string> { std::move(column) }, true);
        }


        Clause count_all();
        Clause count_of(std::string column);
        Clause count_distinct(std::string column);

        Clause min_of(std::string column);
        Clause max_of(std::string column);
        Clause sum_of(std::string column);
        Clause avg_of(std::string column);

        Clause array_agg_of(std::string column);
        Clause bool_and_of(std::string column);
        Clause bool_or_of(std::string column);

        Clause stddev_of(std::string column);
        Clause variance_of(std::string column);

        Clause corr_of(std::string y, std::string x);
    }
}


// StringAggClause

namespace dbio::impl {
    // string_agg(col, <delimiter>) - the delimiter is a value, not an identifier, so it binds
    // as a parameter instead of being spliced into the text.
    class ERSLIB_EXPORT StringAggClause : public IClause {
    public:
        // Member functions

        StringAggClause(std::string column, binder_t delimiter);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        binder_t _delimiter;
    };


    namespace clauses {
        Clause string_agg(std::string column, std::string delimiter);
    }
}


// OrderedSetAggClause

namespace dbio::impl {
    // fn(<arg>) WITHIN GROUP (ORDER BY col[, col...]) - percentile_cont, percentile_disc, mode.
    // mode() takes no argument, hence the optional.
    class ERSLIB_EXPORT OrderedSetAggClause : public IClause {
    public:
        // Member functions

        OrderedSetAggClause(std::string_view fn, ers::optional<binder_t> arg, std::vector<std::string> order_by);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string_view _fn;
        ers::optional<binder_t> _arg;
        std::vector<std::string> _order_by;
    };


    namespace clauses {
        Clause percentile_cont(double fraction, std::vector<std::string> order_by);
        Clause percentile_disc(double fraction, std::vector<std::string> order_by);

        Clause mode_of(std::vector<std::string> order_by);
    }
}


// JsonRowsClause

namespace dbio::impl {
    // json_agg(row_to_json(<table>) [ORDER BY col[, col...]]) - the ordering lives inside the
    // aggregate, so it cannot be expressed with the statement-level order_by() clause.
    class ERSLIB_EXPORT JsonRowsClause : public IClause {
    public:
        // Member functions

        JsonRowsClause(std::string table, std::vector<std::string> order_by);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _table;
        std::vector<std::string> _order_by;
    };


    namespace clauses {
        Clause json_rows(std::string table, std::vector<std::string> order_by = {});
    }
}
