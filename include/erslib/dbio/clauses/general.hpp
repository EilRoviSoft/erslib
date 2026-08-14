#pragma once

// std
#include <concepts>
#include <string>
#include <utility>

// ers
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/impl/query.hpp>


namespace dbio::impl {
    inline constexpr Slot column_slot {
        .name      = "dbio.column",
        .prefix    = "SELECT ",
        .separator = ", ",
        .fallback  = "*",
    };

    inline constexpr Slot from_slot {
        .name      = "dbio.from",
        .prefix    = "\nFROM ",
        .separator = ", ",
    };


    namespace clauses {
        Query select_from(std::string table);

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
