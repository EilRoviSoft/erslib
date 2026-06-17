#pragma once

// std
#include <concepts>
#include <string>

// aescript
#include <aescript/impl/layout.hpp>
#include <aescript/impl/verifier.hpp>


namespace aescript::internal {
    template<typename T>
    concept HasLayoutDescriptor = requires() {
        { T::get_layout() } -> std::same_as<Layout>;
    };


    template<typename T>
    concept SolInnateType =
        std::same_as<T, std::string>
        || std::same_as<T, bool>
        || std::integral<T>
        || std::floating_point<T>;

    template<SolInnateType T>
    consteval sol::type sol_type_for() {
        if constexpr (std::same_as<T, bool>)
            return sol::type::boolean;
        else if constexpr (std::same_as<T, std::string>)
            return sol::type::string;
        else if constexpr (std::floating_point<T> || std::integral<T>)
            return sol::type::number;
        else
            return sol::type::nil;
    }


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
            const Layout& layout = T::get_layout();
            return layout.verify(table[field]);
        }

        template<internal::SolInnateType T>
        static ers::Status _verify_one(sol::table table, std::string_view field) {
            constexpr sol::type expected = internal::sol_type_for<T>();
            sol::object v = table[field];

            if (!v.valid()) {
                return ers::make_error(
                    ers::Severity::Error,
                    "Field '{}' is not valid or nil",
                    field
                );
            }

            if (v.get_type() != expected) {
                return ers::make_error(
                    ers::Severity::Error,
                    "Field '{}' has unexpected type",
                    field
                );
            }

            return ers::ok;
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
