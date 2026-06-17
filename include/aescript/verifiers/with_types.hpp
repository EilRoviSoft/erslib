#pragma once

// aescript
#include <aescript/impl/trait.hpp>
#include <aescript/impl/trait.hpp>
#include <aescript/impl/verifier.hpp>


namespace aescript::internal {
    template<typename T>
    concept FieldType = HasLayoutDescriptor<T> || SolInnateType<T>;
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
            ers::Status result = ers::ok;
            ((result = _verify_one<Ts>(table, field)) && ...);
            return result;
        }


        // Misc

        VerifierPtr clone() const override {
            return std::make_unique<WithTypesProperty>();
        }


    private:
        template<internal::HasLayoutDescriptor T>
        static ers::Status _verify_one(sol::table table, std::string_view field) {
            return T::get_layout().verify(table[field]);
        }

        template<internal::SolInnateType T>
        static ers::Status _verify_one(sol::table table, std::string_view field) {
            return internal::check_type<T>(table[field]);
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
