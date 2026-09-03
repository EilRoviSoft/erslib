#pragma once

// std
#include <cstdint>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/conduit/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    class ERSLIB_EXPORT BoundClause : public IClause {
    public:
        // Member functions

        BoundClause(SlotRef slot, i64 amount);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        i64 _amount;
    };


    namespace clauses {
        Clause with_limit(i64 amount);
        Clause with_offset(i64 amount);
    }
}
