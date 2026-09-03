#pragma once

// std
#include <concepts>
#include <type_traits>
#include <utility>
#include <vector>

// ers
#include <erslib/conduit/impl/clause.hpp>


namespace conduit::impl::clauses {
    // Conditionally include a clause without breaking out of the pipe chain.
    inline std::vector<Clause> when(bool condition, Clause clause) {
        std::vector<Clause> out;

        if (condition)
            out.emplace_back(std::move(clause));

        return out;
    }

    inline std::vector<Clause> when(bool condition, std::vector<Clause> clauses) {
        if (!condition)
            return {};

        return clauses;
    }

    // Lazy form - nothing is built when the condition fails,
    // which matters when the skipped branch is a whole subquery.
    template<typename Fn>
        requires std::invocable<Fn>
    std::vector<Clause> when(bool condition, Fn&& make) {
        if (!condition)
            return {};

        using result_t = std::remove_cvref_t<std::invoke_result_t<Fn>>;

        if constexpr (std::same_as<result_t, std::vector<Clause>>) {
            return std::forward<Fn>(make)();
        } else {
            std::vector<Clause> out;
            out.emplace_back(std::forward<Fn>(make)());
            return out;
        }
    }
}
