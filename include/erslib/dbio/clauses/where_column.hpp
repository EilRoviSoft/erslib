#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/impl/op.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT WhereColumnClause : public IWhereClause {
    public:
        // Member functions

        WhereColumnClause(std::string left, std::string right, Op op);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _left;
        std::string _right;
        Op _op;
    };


    namespace clauses {
        Clause where_column(std::string left, Op op, std::string right);

        Clause where_column(std::string left, std::string right);
    }
}
