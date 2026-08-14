#pragma once

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT WhereClause : public IClause {
    public:
        // Executors

        ers::Status render(Context& ctx) const override;
    };
}
