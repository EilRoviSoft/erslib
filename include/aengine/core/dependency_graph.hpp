#pragma once

// erslib
#include <erslib/type/result.hpp>

// aengine
#include <aengine/core/mod.hpp>
#include <aengine/fwd.hpp>


namespace aengine {
    class DependencyGraph {
        enum class VisitState {
            NonVisited,
            Visiting,
            Visited
        };


    public:
        // Modifiers of initial state

        void add_initial_mod(std::string_view name);


        // Executors

        void resolve_order(const ModContainer& mods);


    protected:
        std::list<std::string> m_initial_mods;
        std::list<std::string> m_execution_order;
        StringMap<VisitState> m_states;
    };
}
