#pragma once

// aescript
#include <aescript/impl/verifier.hpp>


namespace aescript {
    class PresenceProperty : public IVerifier {
    public:
        // Member functions

        explicit PresenceProperty(bool is_required);


        // Checkers

        [[nodiscard]]
        ers::Status exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const override;

        // Misc

        VerifierPtr clone() const override;


    private:
        bool _is_required;
    };


    namespace properties {
        VerifierPtr presence(bool flag);
    }
}
