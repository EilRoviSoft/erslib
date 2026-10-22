#pragma once

// std
#include <string>
#include <utility>

// ers
#include <erslib/flux/query/clause.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    class ERSLIB_EXPORT IdentityClause : public IClause {
    public:
        // Member functions

        IdentityClause(SlotRef slot, std::string name, std::string alias = "");


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _name;
        std::string _alias;
    };


    namespace clauses {
        ERSLIB_EXPORT Clause identifier(SlotRef slot, std::string name, std::string alias = "");


        ERSLIB_EXPORT Clause from(std::string name, std::string alias = "");
    }
}
