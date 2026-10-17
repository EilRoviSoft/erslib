#pragma once

// std
#include <string>

// ers
#include <erslib/flux/clauses/where.hpp>
#include <erslib/flux/impl/op.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT WhereColumnClause : public IWhereClause {
    public:
        // Member functions

        WhereColumnClause(std::string left, std::string right, EOp op);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _left;
        std::string _right;
        EOp _op;
    };


    namespace clauses {
        Clause where_column(std::string left, EOp op, std::string right);
    }
}
