#pragma once

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT BoundClause : public IClause {
    public:
        // Member functions

        BoundClause(Section section, int64_t amount);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        int64_t _amount;
    };


    namespace clauses {
        ClausePtr with_limit(int64_t amount);
        ClausePtr with_offset(int64_t amount);
    }
}
