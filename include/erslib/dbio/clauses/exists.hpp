#pragma once

// ers
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT ExistsClause : public IWhereClause {
    public:
        // Member functions

        ExistsClause(QueryBuilder subquery, bool negated);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        QueryBuilder _subquery;
        bool _negated;
    };


    namespace clauses {
        Clause where_exists(QueryBuilder subquery, bool negated = false);
    }
}
