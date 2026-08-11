#pragma once

// std
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    // Verbatim SQL with '?' value markers.
    // Nothing in the text is validated, so keep untrusted input in the markers, never in the text itself.
    class ERSLIB_EXPORT RawClause : public IClause {
    public:
        // Member functions

        RawClause(Section section, std::string sql, std::vector<binder_t> binders);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::string _sql;
        std::vector<binder_t> _binders;
    };


    namespace clauses {
        template<typename... Args>
        ClausePtr raw(Section section, std::string sql, Args&&... args) {
            std::vector<binder_t> binders;
            binders.reserve(sizeof...(Args));

            (binders.emplace_back(make_binder(std::forward<Args>(args))), ...);

            return std::make_unique<RawClause>(section, std::move(sql), std::move(binders));
        }

        template<typename... Args>
        ClausePtr where_raw(std::string sql, Args&&... args) {
            return raw(section::where, std::move(sql), std::forward<Args>(args)...);
        }
    }
}
