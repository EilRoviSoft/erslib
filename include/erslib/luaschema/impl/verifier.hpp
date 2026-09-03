#pragma once

// std
#include <memory>

// sol
#include <sol/sol.hpp>

// ers
#include <erslib/luaschema/impl/context.hpp>
#include <erslib/core/type/result.hpp>


namespace ers::luaschema::impl {
    using Verifier = std::polymorphic<class IVerifier>;

    class IVerifier {
    public:
        // Member functions

        explicit IVerifier(size_t precedence) : _precedence(precedence) {}
        virtual ~IVerifier() = default;


        // Checkers

        virtual Status exec(verify_context& ctx, sol::table table, std::string_view field) const = 0;


        // Accessors

        size_t precedence() const { return _precedence; }


    private:
        size_t _precedence;
    };


    template<typename T, typename... Args>
    Verifier make_verifier(Args&&... args) {
        return Verifier(std::in_place_type<T>, std::forward<Args>(args)...);
    }
}


namespace ers::luaschema {
    template<typename T>
    struct verifier_t {
        Status exec(impl::verify_context& ctx, sol::object obj) const = delete("no verifier_t specialization for T");
    };
}
