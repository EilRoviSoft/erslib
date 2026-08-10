#pragma once

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio {
    class ERSLIB_EXPORT NullClause : public IClause {
    public:
        // Member functions

        NullClause(std::string column, bool is_null);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::string _column;
        bool _is_null;
    };


    namespace clauses {
        ClausePtr where_null(std::string column, bool is_null = true);
    }
}
