#pragma once

// std
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio {
    class ERSLIB_EXPORT ValuesClause : public IClause {
    public:
        // Member functions

        explicit ValuesClause(std::vector<internal::binder_t> binders);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::vector<internal::binder_t> _binders;
    };


    namespace clauses {
        template<typename... Args>
        ClausePtr values(Args&&... args) {
            std::vector<internal::binder_t> binders;
            binders.reserve(sizeof...(Args));
            (binders.emplace_back(internal::make_binder(std::forward<Args>(args))), ...);
            return std::make_unique<ValuesClause>(std::move(binders));
        }
    }
}
