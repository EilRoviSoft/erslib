#pragma once

// std
#include <concepts>
#include <string>
#include <utility>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT IdentityClause : public IClause {
    public:
        // Member functions

        IdentityClause(SlotRef slot, std::string name);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _name;
    };


    namespace clauses {
        ClausePtr identifier(SlotRef slot, std::string name);


        ClausePtr column(std::string name);

        template<typename... Args>
            requires (std::convertible_to<Args, std::string> && ...)
        ClauseList columns(Args&&... args) {
            ClauseList out;
            out.reserve(sizeof...(Args));
            (out.emplace_back(column(std::forward<Args>(args))), ...);
            return out;
        }


        ClausePtr from(std::string name);
    }
}
