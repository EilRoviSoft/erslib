#pragma once

// std
#include <memory>

// sol
#include <sol/sol.hpp>

// ers
#include <erslib/aescript/impl/context.hpp>
#include <erslib/core/type/result.hpp>


namespace aescript::impl {
    using Parser = std::polymorphic<class IParser>;

    class IParser {
    public:
        // Member functions

        explicit IParser() = default;
        virtual ~IParser() = default;


        // Checkers

        virtual ers::Status exec([[maybe_unused]] parser_context& ctx, sol::table table, std::string_view field, void* dst) const = 0;
    };


    template<typename T, typename... Args>
    Parser make_parser(Args&&... args) {
        return Parser(std::in_place_type<T>, std::forward<Args>(args)...);
    }
}


namespace aescript {
    template<typename T>
    struct parser_t {
        ers::Status exec(
            [[maybe_unused]] impl::parser_context& ctx,
            sol::object obj,
            T& dst
        ) const = delete("generic specialization");
    };
}
