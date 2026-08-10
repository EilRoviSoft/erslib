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
        explicit Query(std::string table);

        Query(const Query& other);
        Query& operator=(const Query& other);

        Query(Query&&) = default;
        Query& operator=(Query&&) = default;


        // Accessors

        [[nodiscard]]
        std::string_view table() const noexcept { return _table; }


        // Modifiers

        void set_table(std::string table);

        void add(ClausePtr clause);


        // Executors

        // Renders into 'ctx'. Public so a query can be spliced into a bigger statement.
        [[nodiscard]]
        ers::Status build(build_context_t& ctx) const;

        // Renders the text only, dropping the bound values. Intended for tests and logging.
        [[nodiscard]]
        ers::Result<std::string> to_sql() const;

        [[nodiscard]]
        ers::Result<pqxx::result> exec(pqxx::dbtransaction& tx) const;

        // Reads rows positionally, so it is only valid while the projection stays '*'.
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
        std::string _table;
        std::vector<ClausePtr> _clauses;


        [[nodiscard]]
        bool _has(Section section) const;

        [[nodiscard]]
        ers::Status _render(build_context_t& ctx, Section section) const;

        void _copy_from(const Query& other);
    };


    Query from(std::string table);
}


// Operators

namespace dbio {
    Query& operator|(Query& lhs, ClausePtr rhs);
    Query&& operator|(Query&& lhs, ClausePtr rhs);
}
