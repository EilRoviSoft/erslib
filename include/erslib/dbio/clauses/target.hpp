#pragma once

// std
#include <concepts>
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio {
    class ERSLIB_EXPORT StClause : public IClause {
    public:
        // Member functions

        StClause(Section section, std::string name);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::string _name;
    };

    class ERSLIB_EXPORT MtClause : public IClause {
    public:
        // Member functions

        MtClause(Section section, std::vector<std::string> names, bool wrap);


        // Executors

        [[nodiscard]]
        ers::Status render(build_context_t& ctx) const override;


        // Misc

        [[nodiscard]]
        ClausePtr clone() const override;


    private:
        std::vector<std::string> _names;
        bool _wrap;
    };


    namespace impl {
        template<bool Wrap, typename... Args>
            requires (std::convertible_to<Args, std::string> && ...)
        auto make_mt_clause(Args&&... args) {
            std::vector<std::string> names;
            names.reserve(sizeof...(Args));
            ((names.emplace_back(std::forward<Args>(args))), ...);
            return std::make_unique<MtClause>(section::column, std::move(names), Wrap);
        }
    }


    namespace clauses {
        ClausePtr column(std::string name);
        
        ClausePtr all_columns();

        template<typename... Args>
            requires (std::convertible_to<Args, std::string> && ...)
        ClausePtr source(Args&&... args) {
            return impl::make_mt_clause<false, Args...>(std::forward<Args>(args)...);
        }

        template<typename... Args>
            requires (std::convertible_to<Args, std::string> && ...)
        ClausePtr dest(Args&&... args) {
            return impl::make_mt_clause<true, Args...>(std::forward<Args>(args)...);
        }
    }
}
