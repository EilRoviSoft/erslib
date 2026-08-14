#pragma once

// std
#include <string>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT IdentityClause : public IClause {
    public:
        // Member functions

        IdentityClause(SlotRef slot, std::string name);


        // Executors

        ers::Status render(Context& ctx) const override;


        // Misc

        ClausePtr clone() const override;


    private:
        std::string _name;
    };


    namespace clauses {
        ClausePtr identifier(SlotRef slot, std::string name);
    }
}
