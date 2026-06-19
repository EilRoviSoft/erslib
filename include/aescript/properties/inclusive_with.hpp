#pragma once

// std
#include <initializer_list>
#include <string>
#include <vector>

// aescript
#include <aescript/impl/verifier.hpp>


namespace aescript {
    class InclusiveWithProperty : public IVerifier {
    public:
        // Member functions

        explicit InclusiveWithProperty(std::vector<std::string> mandatory_fields);


        // Executors

        [[nodiscard]]
        ers::Status exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const override;


        // Misc

        VerifierPtr clone() const override;


    private:
        std::vector<std::string> _mandatory_fields;
    };


    namespace properties {
        VerifierPtr inclusive_with(std::string_view field);
        VerifierPtr inclusive_with(std::initializer_list<std::string_view> il);
    }
}
