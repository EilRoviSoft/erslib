#pragma once

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio {
    class ERSLIB_EXPORT ColumnClause : public IClause {
    public:
        // Member functions

        explicit ColumnClause(std::string name);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::string _name;
    };


    namespace clauses {
        ClausePtr column(std::string name);
    }
}
