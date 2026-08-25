#pragma once

// ers
#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/op.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT JoinClause : public IClause {
    public:
        // Member functions

        JoinClause(std::string table, std::string alias, std::string left, std::string right, Op op);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _table, _alias, _left, _right;
        Op _op;
    };


    namespace clauses {
        Clause join(std::string table, std::string alias, std::string left, Op op, std::string right);

        Clause join(std::string table, std::string alias, std::string left, std::string right);
    }
}
