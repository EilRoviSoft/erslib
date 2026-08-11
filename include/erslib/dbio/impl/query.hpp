#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/dbio/generator.hpp>
#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/context.hpp>

// export
#include <erslib/export.hpp>


// Query

namespace dbio {
    class ERSLIB_EXPORT Query {
    public:
        // Member functions

        Query() = default;

        Query(const Query& other);
        Query& operator=(const Query& other);

        Query(Query&&) = default;
        Query& operator=(Query&&) = default;


        // Modifiers

        void add(ClausePtr clause);


        // Executors

        [[nodiscard]]
        ers::Status build(build_context_t& ctx) const;

        [[nodiscard]]
        ers::Result<std::string> to_sql() const;

        [[nodiscard]]
        ers::Result<pqxx::result> exec(pqxx::dbtransaction& tx) const;

        // Same as exec(), but discards the pqxx::result - for statements with nothing
        // meaningful to read back (e.g. a mass insert with no RETURNING).
        [[nodiscard]]
        ers::Status exec_and_discard(pqxx::dbtransaction& tx) const;

        template<typename T>
            requires ValidRow<T>
        [[nodiscard]]
        ers::Result<RowGenerator<T>> exec_as(pqxx::dbtransaction& tx) const {
            auto r = exec(tx);
            if (!r)
                return r.error();

            return RowGenerator<T>(std::move(*r));
        }

        template<typename T>
            requires ValidRow<T>
        [[nodiscard]]
        ers::Result<T> exec_one(pqxx::dbtransaction& tx) const {
            auto r = exec(tx);
            if (!r)
                return r.error();

            if (r->size() != 1)
                return ers::make_error("dbio::Query: expected exactly 1 row, got {}", r->size());

            return T(r->one_row_ref());
        }


    private:
        std::vector<ClausePtr> _clauses;


        [[nodiscard]]
        ers::Status _render(build_context_t& ctx, Section section) const;

        void _copy_from(const Query& other);
    };
}


// Operators

namespace dbio {
    Query& operator|(Query& lhs, ClausePtr rhs);
    Query&& operator|(Query&& lhs, ClausePtr rhs);
    Query& operator|=(Query& lhs, ClausePtr rhs);
}
