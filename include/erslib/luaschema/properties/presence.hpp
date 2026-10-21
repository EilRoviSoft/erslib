#pragma once

// std
#include <string_view>

// ers
#include <erslib/luaschema/impl/verifier.hpp>

// export
#include <erslib/export.hpp>


namespace ers::luaschema::impl {
    class ERSLIB_EXPORT PresenceProperty : public IVerifier {
    public:
        // Member functions

        explicit PresenceProperty(bool is_required);


        // Executors

        Status exec(verify_context& ctx, sol::table table, std::string_view field) const override;


    private:
        bool _is_required;
    };


    namespace properties {
        ERSLIB_EXPORT Verifier presence(bool flag);
    }
}
