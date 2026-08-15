#pragma once

// std
#include <initializer_list>
#include <string>
#include <vector>

// ers
#include <erslib/aescript/impl/verifier.hpp>


namespace aescript::impl {
    class InclusiveWithProperty : public IVerifier {
    public:
        // Member functions

        explicit InclusiveWithProperty(std::vector<std::string> mandatory_fields);


        // Executors

        ers::Status exec([[maybe_unused]] verify_context& ctx, sol::table table, std::string_view field) const override;


    private:
        std::vector<std::string> _mandatory_fields;
    };


    namespace properties {
        Verifier inclusive_with(std::string_view field);
        Verifier inclusive_with(std::initializer_list<std::string_view> il);
    }
}
