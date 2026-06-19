#pragma once

// sol
#include <sol/state_view.hpp>

// aengine
#include <erslib/aengine/mod.hpp>


namespace aengine {
    class IPhaseContext {
    public:
        // Member functions

        virtual ~IPhaseContext() = default;


        // Modifiers

        virtual void inject(sol::state_view& lua) = 0;
        virtual void inject_per_mod(sol::state_view& lua, const Mod& mod) {}

        virtual void cleanup(sol::state_view& lua) = 0;
    };
}
