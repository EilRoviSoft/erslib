#pragma once

// std
#include <initializer_list>
#include <string>
#include <vector>

// ers
#include <erslib/luaschema/impl/verifier.hpp>


namespace ers::luaschema::impl {
    class ExclusiveWithProperty : public IVerifier {
    public:
        // Member functions

        explicit ExclusiveWithProperty(std::vector<std::string> incompatible_fields);


        // Executors

        Status exec(verify_context& ctx, sol::table table, std::string_view field) const override;


    private:
        std::vector<std::string> _incompatible_fields;
    };


    namespace properties {
        Verifier exclusive_with(std::string_view field);
        Verifier exclusive_with(std::initializer_list<std::string_view> il);
    }
}
