#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/context.hpp>

// export
#include <erslib/export.hpp>


// Query

namespace dbio {
    class ERSLIB_EXPORT Query {
    private:
        std::string _table;
        std::vector<ClausePtr> _clauses;


        [[nodiscard]]
        bool _has(Section section) const;

        [[nodiscard]]
        ers::Status _render(build_context_t& ctx, Section section) const;

        void _copy_from(const Query& other);
    };


    Query from(std::string table);
}


// Operators

namespace dbio {
    Query& operator|(Query& lhs, ClausePtr rhs);
    Query&& operator|(Query&& lhs, ClausePtr rhs);
}
