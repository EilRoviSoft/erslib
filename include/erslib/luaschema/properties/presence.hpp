#pragma once

// ers
#include <erslib/luaschema/impl/verifier.hpp>


namespace ers::luaschema::impl {
    class PresenceProperty : public IVerifier {
    public:
        // Member functions

        explicit PresenceProperty(bool is_required);


        // Executors

        Status exec(verify_context& ctx, sol::table table, std::string_view field) const override;


    private:
        bool _is_required;
    };


    namespace properties {
        Verifier presence(bool flag);
    }
}
