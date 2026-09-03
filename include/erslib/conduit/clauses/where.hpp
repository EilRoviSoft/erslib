#pragma once

// ers
#include <erslib/conduit/impl/clause.hpp>

// export
#include <erslib/export.hpp>


// IWhere

namespace conduit::impl {
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
