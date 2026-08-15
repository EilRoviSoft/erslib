#pragma once

// std
#include <memory>

// sol
#include <sol/sol.hpp>

// ers
#include <erslib/aescript/impl/context.hpp>
#include <erslib/core/type/result.hpp>


namespace aescript::impl {
    using Verifier = std::polymorphic<class IVerifier>;

    class IVerifier {
    public:
        // Member functions

        explicit IVerifier(size_t precedence) : _precedence(precedence) {}
        virtual ~IVerifier() = default;


        // Checkers

        virtual ers::Status exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const = 0;


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


// verifier_t<T> is a customization point, same rationale as parser_t<T> in impl/parser.hpp:
// every file under aescript/verifiers/ explicitly specializes it, so it stays in aescript::.

namespace aescript {
    template<typename T>
    struct verifier_t {
        ers::Status exec(impl::verify_context& ctx, sol::object obj) const = delete("no verifier_t specialization for T");
    };
}
