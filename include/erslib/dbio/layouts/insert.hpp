#pragma once

// std
#include <concepts>
#include <string>

// ers
#include <erslib/dbio/impl/query.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    namespace slots {
        inline constexpr Slot insert_into = {
            .name   = "dbio.insert_into",
            .prefix = "INSERT INTO ",
            .arity  = Arity::Single,
        };

        inline constexpr Slot insert_columns {
            .name      = "dbio.insert_columns",
            .prefix    = " (",
            .separator = ", ",
            .suffix    = ")",
            .arity     = Arity::Multi,
        };
    }


    namespace clauses {
        ClausePtr into_column(std::string name);

        template<typename... Args>
            requires (std::convertible_to<Args, std::string> && ...)
        ClauseList into_columns(Args&&... args) {
            ClauseList out;
            out.reserve(sizeof...(Args));
            (out.emplace_back(into_column(std::forward<Args>(args))), ...);
            return out;
        }
    }


    namespace layouts {
        ERSLIB_EXPORT Query insert_into(std::string table);
    }
}
