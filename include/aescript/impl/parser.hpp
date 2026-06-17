#pragma once

// sol
#include <sol/sol.hpp>

// ers
#include <erslib/type/result.hpp>

// aescript
#include <aescript/impl/context.hpp>


namespace aescript {
    using ParserPtr = std::unique_ptr<class IParser>;

    class IParser {
    public:
        // Member functions

        explicit IParser() = default;
        virtual ~IParser() = default;


        // Checkers

        [[nodiscard]]
        virtual ers::Status exec([[maybe_unused]] parser_context& ctx, sol::table table, std::string_view field, void* dst) const = 0;


        // Misc

        [[nodiscard]]
        virtual ParserPtr clone() const = 0;
    };
}
