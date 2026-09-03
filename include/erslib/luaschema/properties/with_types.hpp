#pragma once

// ers
#include <erslib/luaschema/impl/trait.hpp>
#include <erslib/luaschema/impl/verifier.hpp>


namespace ers::luaschema::impl {
    template<typename T>
    concept FieldType = HasLayoutDescriptor<T> || SolInnateType<T> || HasVerifier<T>;
}


namespace ers::luaschema::impl {
    template<FieldType... Ts>
    class WithTypesProperty : public IVerifier {
    public:
        // Member functions

        WithTypesProperty() : IVerifier(2) {}


        // Executors

        Status exec(verify_context& ctx, sol::table table, std::string_view field) const override {
            sol::object obj = table[field];
            Status result = ok;
            ((result = verifier_t<Ts> {}.exec(ctx, obj)) && ...);
            return result;
        }
    };


    namespace properties {
        template<FieldType T>
        Verifier with_type() {
            return make_verifier<WithTypesProperty<T>>();
        }

        template<FieldType... Ts>
        Verifier with_types() {
            return make_verifier<WithTypesProperty<Ts...>>();
        }
    }
}
