#pragma once

// std
#include <concepts>
#include <ranges>
#include <vector>

// ers
#include <erslib/conduit/impl/clause.hpp>


namespace conduit::impl {
    namespace clauses {
        Clause returning(std::string name);

        Clause returning_all();


        template<typename... Args>
            requires(sizeof...(Args) >= 2 && (std::convertible_to<Args, std::string> && ...))
        auto returning(Args&&... args) {
            std::vector<Clause> out;
            out.reserve(sizeof...(Args));
            (out.emplace_back(returning(std::forward<Args>(args))), ...);
            return out;
        }

        template<std::ranges::input_range R>
            requires std::constructible_from<std::string, std::ranges::range_reference_t<R>>
        auto returning(R&& names) {
            std::vector<Clause> out;

            if constexpr (std::ranges::sized_range<R>)
                out.reserve(std::ranges::size(names));

            for (auto&& it : names)
                out.emplace_back(returning(std::string(std::forward<decltype(it)>(it))));

            return out;
        }
    }
}
