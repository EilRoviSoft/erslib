#pragma once

// std
#include <concepts>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/dbio/clauses/raw.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/impl/clause.hpp>
#include <erslib/dbio/impl/identity.hpp>
#include <erslib/dbio/slots/conflict.hpp>

// export
#include <erslib/export.hpp>


// ConflictUpdateClause

namespace dbio::impl {
    // Renders the whole action: "DO UPDATE SET a = excluded.a, b = excluded.b".
    class ERSLIB_EXPORT ConflictUpdateClause : public IClause {
    public:
        // Member functions

        explicit ConflictUpdateClause(std::vector<std::string> columns) :
            IClause(&slots::conflict_action),
            _columns(std::move(columns)) {
        }


        // Executors

        ers::Status render(Context& ctx) const override {
            if (_columns.empty())
                return ers::make_error("Slot '{}' needs at least one column.", slot()->name);

            ctx.query += "DO UPDATE SET ";

            bool first = true;

            for (const std::string& it : _columns) {
                if (!is_identifier(it)) {
                    return ers::make_error("Invalid identifier '{}' in slot '{}'.",
                        it, slot()->name);
                }

                if (!first)
                    ctx.query += ", ";
                first = false;

                ctx.query += it;
                ctx.query += " = excluded.";
                ctx.query += it;
            }

            return ers::ok;
        }


    private:
        std::vector<std::string> _columns;
    };
}


// Shortcuts

namespace dbio::impl::clauses {
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


    inline Clause do_nothing() {
        return raw(&slots::conflict_action, "DO NOTHING");
    }

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
