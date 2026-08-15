#pragma once

// std
#include <utility>
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT ValuesClause : public IClause {
    public:
        // Member functions

        explicit ValuesClause(std::vector<binder_t> binders);


        // Executors

        [[nodiscard]]
        ers::Status render(Context& ctx) const override;


    private:
        std::vector<binder_t> _binders;
    };


    namespace clauses {
        template<typename... Args>
        ClausePtr values(Args&&... args) {
            std::vector<binder_t> binders;
            binders.reserve(sizeof...(Args));
            (binders.emplace_back(make_binder(std::forward<Args>(args))), ...);
            return make_clause<ValuesClause>(std::move(binders));
        }
    }
}
