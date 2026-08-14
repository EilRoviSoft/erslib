#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/dbio/generator.hpp>
#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/context.hpp>
#include <erslib/dbio/impl/layout.hpp>

// export
#include <erslib/export.hpp>


// Query

namespace dbio::impl {
    class ERSLIB_EXPORT Query {
    public:
        // Member functions

        explicit Query(SlotView layout) :
            _layout(layout) {
        }

        Query(const Query& other);
        Query& operator=(const Query& other);

        Query(Query&&) = default;
        Query& operator=(Query&&) = default;

        ~Query() = default;


        // Accessors

        Layout& layout() noexcept { return _layout; }
        const Layout& layout() const noexcept { return _layout; }


        // Modifiers

        void add(ClausePtr clause);
        void add(ClauseList clauses);


        ers::Status build(Context& ctx) const;

        ers::Result<std::string> to_sql() const;


        // Executors

        ers::Result<pqxx::result> exec(pqxx::dbtransaction& tx) const;

        ers::Status exec_and_discard(pqxx::dbtransaction& tx) const;

        template<typename T>
            requires ValidRow<T>
        ers::Result<RowGenerator<T>> exec_as(pqxx::dbtransaction& tx) const {
            auto r = exec(tx);
            if (!r)
                return r.error();

            return RowGenerator<T>(std::move(*r));
        }

        template<typename T>
            requires ValidRow<T>
        ers::Result<T> exec_one(pqxx::dbtransaction& tx) const {
            auto r = exec(tx);
            if (!r)
                return r.error();

            if (r->size() != 1)
                return ers::make_error("Expected exactly 1 row, got {}.", r->size());

            return T(r->one_row_ref());
        }


    private:
        Layout _layout;
        ClauseList _clauses;


        ers::Status _render_slot(Context& ctx, SlotRef slot) const;
        ers::Status _render_custom(Context& ctx, SlotRef slot) const;


        ers::Status _validate() const;


        void _copy_from(const Query& other);
    };
}


// Operators

namespace dbio::impl {
    Query& operator|(Query& lhs, ClausePtr rhs);
    Query&& operator|(Query&& lhs, ClausePtr rhs);
    Query& operator|=(Query& lhs, ClausePtr rhs);

    Query& operator|(Query& lhs, ClauseList rhs);
    Query&& operator|(Query&& lhs, ClauseList rhs);
    Query& operator|=(Query& lhs, ClauseList rhs);
}
