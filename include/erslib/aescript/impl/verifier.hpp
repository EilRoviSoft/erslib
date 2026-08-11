#pragma once

// sol
#include <sol/sol.hpp>

// ers
#include <erslib/aescript/impl/context.hpp>
#include <erslib/core/type/result.hpp>


namespace aescript::impl {
    // TODO: add 'make_verifier' instead of 'make_unique<DerivedFromVerifier>'
    using VerifierPtr = std::unique_ptr<class IVerifier>;

    class IVerifier {
    public:
        // Member functions

        explicit IVerifier(size_t precedence) : _precedence(precedence) {}
        virtual ~IVerifier() = default;


        // Checkers

        [[nodiscard]]
        virtual ers::Status exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const = 0;


        // Accessors

        [[nodiscard]]
        size_t precedence() const { return _precedence; }


        // Misc

        [[nodiscard]]
        virtual VerifierPtr clone() const = 0;


    private:
        size_t _precedence;
    };
}


// verifier_t<T> is a customization point, same rationale as parser_t<T> in impl/parser.hpp:
// every file under aescript/verifiers/ explicitly specializes it, so it stays in aescript::.

namespace aescript {
    template<typename T>
    struct verifier_t {
        ers::Status exec(verify_context& ctx, sol::object obj) const = delete("no verifier_t specialization for T");
    };
}
