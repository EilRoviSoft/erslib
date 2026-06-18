#pragma once

// aescript
#include <aescript/impl/trait.hpp>
#include <aescript/impl/verifier.hpp>


namespace aescript::internal {
    template<typename T>
    concept FieldType = HasLayoutDescriptor<T> || SolInnateType<T> || HasVerifier<T>;
}


namespace aescript {
    template<internal::FieldType... Ts>
    class WithTypesProperty : public IVerifier {
    public:
        // Member functions

        WithTypesProperty() : IVerifier(2) {}


        // Checkers

        [[nodiscard]]
        ers::Status exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const override {
            sol::object obj = table[field];
            ers::Status result = ers::ok;
            ((result = verifier_t<Ts> {}.exec(ctx, obj)) && ...);
            return result;
        }


        // Misc

        VerifierPtr clone() const override {
            return std::make_unique<WithTypesProperty>();
        }
    };


    namespace properties {
        template<internal::FieldType T>
        VerifierPtr with_type() {
            return std::make_unique<WithTypesProperty<T>>();
        }

        template<internal::FieldType... Ts>
        VerifierPtr with_types() {
            return std::make_unique<WithTypesProperty<Ts...>>();
        }
    }
}
