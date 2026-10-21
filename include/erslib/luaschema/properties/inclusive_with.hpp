#pragma once

// std
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/luaschema/impl/verifier.hpp>

// export
#include <erslib/export.hpp>


namespace ers::luaschema::impl {
    class ERSLIB_EXPORT InclusiveWithProperty : public IVerifier {
    public:
        // Member functions

        explicit InclusiveWithProperty(std::vector<std::string> mandatory_fields);


        // Executors

        Status exec(verify_context& ctx, sol::table table, std::string_view field) const override;


    private:
        std::vector<std::string> _mandatory_fields;
    };


    namespace properties {
        ERSLIB_EXPORT Verifier inclusive_with(std::string_view field);
        ERSLIB_EXPORT Verifier inclusive_with(std::initializer_list<std::string_view> il);
    }
}
