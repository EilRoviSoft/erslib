#pragma once

// std
#include <string>

// ers
#include <erslib/flux/query/clause.hpp>
#include <erslib/flux/sql/op.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT JoinClause : public IClause {
    public:
        // Member functions

        JoinClause(std::string table, std::string alias, std::string left, std::string right, EOp op);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _table, _alias, _left, _right;
        EOp _op;
    };


    namespace clauses {
        Clause join(std::string table, std::string alias, std::string left, EOp op, std::string right);
        Clause join(std::string table, std::string left, EOp op, std::string right);
    }
}
