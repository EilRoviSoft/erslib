#pragma once

// std
#include <string>

// ers
#include <erslib/conduit/clauses/where.hpp>


namespace conduit::impl {
    class ERSLIB_EXPORT WhereNullClause : public IWhereClause {
    public:
        // Member functions

        WhereNullClause(std::string column, bool is_null);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _column;
        bool _is_null;
    };


    namespace clauses {
        Clause where_null(std::string column, bool is_null = true);
    }
}
