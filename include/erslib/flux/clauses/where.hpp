#pragma once

// ers
#include <erslib/flux/impl/clause.hpp>

// export
#include <erslib/export.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl {
    class ERSLIB_EXPORT IWhereClause : public IClause {
    public:
        // Member functions

        IWhereClause(SlotRef slot) :
            IClause(slot) {
        }


        // Executors

        ers::Status render(Context& ctx) const override = 0;
    };
}
