#pragma once

// std
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/conduit/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    // Verbatim SQL with '?' value markers.
    // Nothing in the text is validated, so keep untrusted input in the markers, never in the text itself.
    class ERSLIB_EXPORT RawClause : public IClause {
    public:
        // Member functions

        RawClause(SlotRef slot, std::string sql, std::vector<binder_t> binders);


        // Executors

        [[nodiscard]]
        ers::Status render(Context& ctx) const override;


    private:
        std::string _sql;
        std::vector<binder_t> _binders;
    };


    namespace clauses {
        template<typename... Args>
        Clause raw(SlotRef slot, std::string sql, Args&&... args) {
            std::vector<binder_t> binders;
            binders.reserve(sizeof...(Args));
            (binders.emplace_back(make_binder(std::forward<Args>(args))), ...);
            return make_clause<RawClause>(slot, std::move(sql), std::move(binders));
        }
    }
}
