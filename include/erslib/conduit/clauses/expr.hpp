#pragma once

// std
#include <string>

// ers
#include <erslib/conduit/impl/clause.hpp>
#include <erslib/conduit/impl/query_builder.hpp>

// export
#include <erslib/export.hpp>


// Scalar expressions


// AliasClause

namespace conduit::impl {
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

namespace conduit::impl {
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

namespace conduit::impl {
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

namespace conduit::impl::clauses {
    Clause sql_bool(bool value, SlotRef slot = nullptr);
}
