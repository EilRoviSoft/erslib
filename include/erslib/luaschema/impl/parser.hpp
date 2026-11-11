#pragma once

// std
#include <memory>
#include <string_view>
#include <utility>

// sol
#include <sol/sol.hpp>

// ers
#include <erslib/luaschema/impl/context.hpp>
#include <erslib/core/type/result.hpp>


namespace ers::impl::luaschema {
    using Parser = std::polymorphic<class IParser>;

    class IParser {
    public:
        // Member functions

        explicit IParser() = default;
        virtual ~IParser() = default;


        // Validators

        virtual Status exec(parser_context& ctx, sol::table table, std::string_view field, void* dst) const = 0;
    };


    template<typename T, typename... Args>
    Parser make_parser(Args&&... args) {
        return Parser(std::in_place_type<T>, std::forward<Args>(args)...);
    }


    template<typename T>
    struct parser_t {
        Status exec(
            parser_context& ctx,
            sol::object obj,
            T& dst
        ) const = delete("generic specialization");
    };
}


// Exports ---------------------------------------------------------------------

namespace ers::luaschema {
    using impl::luaschema::parser_t;
}
