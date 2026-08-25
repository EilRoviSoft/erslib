#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/dbio/impl/clause.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    class ERSLIB_EXPORT IdentityClause : public IClause {
    public:
        // Member functions

        IdentityClause(SlotRef slot, std::string name, std::string alias = "");


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::string _name;
        std::string _alias;
    };


    namespace clauses {
        Clause identifier(SlotRef slot, std::string name, std::string alias = "");


        Clause column(std::string name);

        template<typename... Args>
            requires (std::convertible_to<Args, std::string> && ...)
        auto columns(Args&&... args) {
            std::vector<Clause> out;
            out.reserve(sizeof...(Args));
            (out.emplace_back(column(std::forward<Args>(args))), ...);
            return out;
        }

        template<std::ranges::input_range R>
            requires std::constructible_from<std::string, std::ranges::range_reference_t<R>>
        auto columns(R&& names) {
            std::vector<Clause> out;

            if constexpr (std::ranges::sized_range<R>)
                out.reserve(std::ranges::size(names));

            for (auto&& it : names)
                out.emplace_back(column(std::string(std::forward<decltype(it)>(it))));

            return out;
        }


        Clause from(std::string name, std::string alias = "");
    }
}
