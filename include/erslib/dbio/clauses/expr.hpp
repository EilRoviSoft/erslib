#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


// Scalar expressions


// AliasClause

namespace dbio::impl {
    // <inner> AS <alias>
    class ERSLIB_EXPORT AliasClause : public IClause {
    public:
        // Member functions

        // The base is initialised before the members, so reading inner's slot here happens
        // while inner is still intact - the alias inherits whatever slot it targets.
        AliasClause(Clause inner, std::string alias);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        Clause _inner;
        std::string _alias;
    };


    namespace clauses {
        Clause named(Clause inner, std::string alias);
    }
}


// CoalesceClause

namespace dbio::impl {
    // coalesce(<inner>, <fallback>)
    class ERSLIB_EXPORT CoalesceClause : public IClause {
    public:
        // Member functions

        CoalesceClause(Clause inner, Clause fallback);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        Clause _inner;
        Clause _fallback;
    };


    namespace clauses {
        Clause coalesce_with(Clause inner, Clause fallback);
    }
}


// SubqueryClause

namespace dbio::impl {
    // (<subquery>) or EXISTS (<subquery>), as a select-list item.
    class ERSLIB_EXPORT SubqueryClause : public IClause {
    public:
        // Member functions

        SubqueryClause(QueryBuilder subquery, bool exists);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        QueryBuilder _subquery;
        bool _exists;
    };


    namespace clauses {
        Clause subquery(QueryBuilder subquery);
        Clause subquery_exists(QueryBuilder subquery);
    }
}


// Literals

namespace dbio::impl::clauses {
    Clause sql_bool(bool value, SlotRef slot = nullptr);
}
