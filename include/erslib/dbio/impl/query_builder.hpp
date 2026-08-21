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
    class ERSLIB_EXPORT QueryBuilder {
    public:
        // Member functions

        explicit QueryBuilder(LayoutView layout) :
            _layout(layout) {
        }


        // Accessors

        Layout& layout() noexcept { return _layout; }
        const Layout& layout() const noexcept { return _layout; }


        // Modifiers

        void add(Clause clause);
        void add(std::vector<Clause> clauses);


        ers::Status build(Context& ctx) const;

        ers::Result<std::string> to_sql() const;


        // Executors

        ers::Result<pqxx::result> exec(pqxx::dbtransaction& tx) const;

        ers::Status exec_and_discard(pqxx::dbtransaction& tx) const;

        template<typename T>
            requires ReadableRow<T>
        ers::Result<RowGenerator<T>> exec_as(pqxx::dbtransaction& tx) const {
            auto r = exec(tx);
            if (!r)
                return r.error();

            return RowGenerator<T>(std::move(*r));
        }

        template<typename T>
            requires ReadableRow<T>
        ers::Result<T> exec_one(pqxx::dbtransaction& tx) const {
            auto r = exec(tx);
            if (!r)
                return r.error();

            if (r->size() != 1)
                return ers::make_error("Expected exactly 1 row, got {}.", r->size());

            return row_reader<T>::read(r->one_row_ref(), row_reader<T>::prepare(*r));
        }


    private:
        Layout _layout;
        std::vector<Clause> _clauses;


        ers::Status _render_slot(Context& ctx, const SlotBinding& binding) const;
        ers::Status _render_custom(Context& ctx, const SlotBinding& binding) const;


        ers::Status _validate() const;
    };
}


// Operators

namespace dbio::impl {
    QueryBuilder& operator|(QueryBuilder& lhs, Clause rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, Clause rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, Clause rhs);

    QueryBuilder& operator|(QueryBuilder& lhs, std::vector<Clause> rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, std::vector<Clause> rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, std::vector<Clause> rhs);
}
