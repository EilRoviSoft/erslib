#pragma once

// std
#include <ranges>
#include <string>
#include <utility>

// ers
#include <erslib/conduit/clauses/where.hpp>
#include <erslib/conduit/impl/binder.hpp>
#include <erslib/conduit/traits.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    // Array-membership filters, in the three shapes that actually come up:
    //
    //   where_any          -> <col> = ANY($n)
    //   where_any_or_empty -> (cardinality($n::<type>[]) = 0 OR <col> = ANY($n))
    //   empty_array        -> cardinality($n::<type>[]) = 0
    class ERSLIB_EXPORT WhereAnyClause : public IWhereClause {
    public:
        // Member functions

        // An empty column means the cardinality check alone.
        WhereAnyClause(std::string column, binder_t values, std::string cast, bool guard_empty);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        binder_t _values;
        std::string _cast;
        bool _guard_empty;
    };


    namespace clauses {
        namespace detail {
            template<std::ranges::input_range R>
            std::string array_cast() {
                using value_t = owned_t<std::ranges::range_value_t<R>>;
                return std::string(sql_type<value_t>::name) + "[]";
            }
        }


        template<std::ranges::input_range R>
        Clause where_any(std::string column, R&& values) {
            return make_clause<WhereAnyClause>(std::move(column),
                make_binder(std::forward<R>(values)), detail::array_cast<R>(), false);
        }

        template<std::ranges::input_range R>
        Clause where_any_or_empty(std::string column, R&& values) {
            return make_clause<WhereAnyClause>(std::move(column),
                make_binder(std::forward<R>(values)), detail::array_cast<R>(), true);
        }

        template<std::ranges::input_range R>
        Clause empty_array(R&& values) {
            return make_clause<WhereAnyClause>(std::string(),
                make_binder(std::forward<R>(values)), detail::array_cast<R>(), true);
        }
    }
}
