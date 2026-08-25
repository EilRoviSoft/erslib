#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/context.hpp>
#include <erslib/dbio/impl/generator.hpp>
#include <erslib/dbio/impl/layout.hpp>
#include <erslib/dbio/impl/traits.hpp>

// export
#include <erslib/export.hpp>


// Query

namespace dbio::impl {
    class QueryResult;

    struct WithCte;
    struct WithUnion;
}

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

        void add(WithCte with_cte);
        void add(WithUnion with_union);


        ers::Status build(Context& ctx) const;

        ers::Result<std::string> to_sql() const;


        // Executors

        QueryResult exec(pqxx::dbtransaction& tx) const;


    private:
        Layout _layout;
        std::vector<Clause> _clauses;
        std::vector<WithCte> _ctes;
        std::vector<WithUnion> _unions;


        ers::Status _render_slot(Context& ctx, const SlotBinding& binding) const;
        ers::Status _render_custom(Context& ctx, const SlotBinding& binding) const;


        ers::Status _validate() const;
    };
}


// QueryResult

namespace dbio::impl {
    class QueryResult {
    public:
        explicit QueryResult(pqxx::result content) :
            _content(std::move(content)) {
        }


        const pqxx::result& raw() const noexcept { return _content; }


        // T is a container (including ers::optional<Row>, via sql_collector<optional<T>>) ->
        // eager, 0..N (or 0..1) rows, never throws on row count.
        template<typename T>
            requires RowContainer<T>
        T get_as() const {
            return sql_collector<T>::collect(_content);
        }

        // T is itself the row type -> eager, exactly 1 row; throws otherwise.
        template<typename T>
            requires ReadableRow<T>
        T get_as() const {
            return read_one<T>(_content);
        }

        // Always lazy, always 0..N rows, never throws on row count.
        template<ReadableRow T>
        RowGenerator<T> view_as() const {
            return RowGenerator<T>(_content);
        }


    private:
        pqxx::result _content;
    };
}


// Exclusive clauses

namespace dbio::impl {
    struct WithCte {
        QueryBuilder definition;
        std::string name;
        bool recursive;
    };

    struct WithUnion {
        QueryBuilder other;
        bool all;
    };


    namespace clauses {
        WithCte ERSLIB_EXPORT with_cte(std::string name, QueryBuilder definition, bool recursive = false);

        WithUnion ERSLIB_EXPORT union_with(QueryBuilder other, bool all = false);
    }
}



// Executable

namespace dbio::impl {
    template<typename T>
    concept Executable = requires(const T& q, pqxx::dbtransaction& tx) {
        { q.exec(tx) } -> std::same_as<QueryResult>;
    };

    template<typename T>
    concept ExecutableWithOutput = Executable<T> && requires { typename T::Output; };
}


// Operators

namespace dbio::impl {
    QueryBuilder& operator|(QueryBuilder& lhs, Clause rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, Clause rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, Clause rhs);

    QueryBuilder& operator|(QueryBuilder& lhs, std::vector<Clause> rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, std::vector<Clause> rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, std::vector<Clause> rhs);

    QueryBuilder& operator|(QueryBuilder& lhs, WithUnion rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, WithUnion rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, WithUnion rhs);

    QueryBuilder& operator|(QueryBuilder& lhs, WithCte rhs);
    QueryBuilder&& operator|(QueryBuilder&& lhs, WithCte rhs);
    QueryBuilder& operator|=(QueryBuilder& lhs, WithCte rhs);
}


// Subqueries

namespace dbio::impl {
    ers::Status ERSLIB_EXPORT append_subquery(
        Context& ctx, const QueryBuilder& sub,
        std::string_view open, std::string_view close = ")"
    );
}
