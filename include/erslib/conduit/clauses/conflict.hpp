#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/conduit/clauses/target.hpp>
#include <erslib/conduit/impl/clause.hpp>
#include <erslib/conduit/slots/conflict.hpp>

// export
#include <erslib/export.hpp>


// ConflictUpdateClause

namespace conduit::impl {
    // Renders the whole action: "DO UPDATE SET a = excluded.a, b = excluded.b".
    class ERSLIB_EXPORT ConflictUpdateClause : public IClause {
    public:
        // Member functions

        explicit ConflictUpdateClause(std::vector<std::string> columns);


        // Executors

        ers::Status render(Context& ctx) const override;


    private:
        std::vector<std::string> _columns;
    };
}


// Shortcuts

namespace conduit::impl::clauses {
    inline Clause on_conflict(std::string column) {
        return identifier(&slots::conflict, std::move(column));
    }

    template<typename... Args>
        requires (sizeof...(Args) >= 2 && (std::convertible_to<Args, std::string> && ...))
    auto on_conflict(Args&&... args) {
        std::vector<Clause> out;
        out.reserve(sizeof...(Args));
        (out.emplace_back(identifier(&slots::conflict, std::forward<Args>(args))), ...);
        return out;
    }

    template<std::ranges::input_range R>
        requires std::constructible_from<std::string, std::ranges::range_reference_t<R>>
    auto on_conflict(R&& names) {
        std::vector<Clause> out;

        if constexpr (std::ranges::sized_range<R>)
            out.reserve(std::ranges::size(names));

        for (auto&& it : names)
            out.emplace_back(identifier(&slots::conflict,
                std::string(std::forward<decltype(it)>(it))));

        return out;
    }


    Clause on_conflict_constraint(std::string name);


    Clause do_nothing();

    template<std::ranges::input_range R>
        requires std::constructible_from<std::string, std::ranges::range_reference_t<R>>
    Clause do_update(R&& names) {
        std::vector<std::string> out;

        if constexpr (std::ranges::sized_range<R>)
            out.reserve(std::ranges::size(names));

        for (auto&& it : names)
            out.emplace_back(std::string(std::forward<decltype(it)>(it)));

        return make_clause<ConflictUpdateClause>(std::move(out));
    }

    template<typename... Args>
        requires (sizeof...(Args) >= 1 && (std::convertible_to<Args, std::string> && ...))
    Clause do_update(Args&&... args) {
        std::vector<std::string> out;
        out.reserve(sizeof...(Args));
        (out.emplace_back(std::forward<Args>(args)), ...);
        return make_clause<ConflictUpdateClause>(std::move(out));
    }
}
