#pragma once

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT BoundClause : public IClause {
    public:
        // Member functions

        BoundClause(SlotRef slot, int64_t amount);


        // Executors

        ers::Status render(Context& ctx) const override;


        // Misc

        ClausePtr clone() const override;


    private:
        int64_t amount;
    };


    namespace clauses {
        ClausePtr with_limit(int64_t amount);
        ClausePtr with_offset(int64_t amount);
    }
}
