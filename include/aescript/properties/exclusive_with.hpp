#pragma once

// std
#include <initializer_list>
#include <string>
#include <vector>

// aescript
#include <aescript/impl/verifier.hpp>


namespace aescript {
    class ExclusiveWithProperty : public IVerifier {
    public:
        // Member functions

        explicit ExclusiveWithProperty(std::vector<std::string> incompatible_fields);


        // Executors

        [[nodiscard]]
        ers::Status exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const override;


        // Misc

        VerifierPtr clone() const override;


    private:
        std::vector<std::string> _incompatible_fields;
    };


    namespace properties {
        VerifierPtr exclusive_with(std::string_view field);
        VerifierPtr exclusive_with(std::initializer_list<std::string_view> il);
    }
}
