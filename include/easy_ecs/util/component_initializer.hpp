#pragma once

// std
#include <list>

// sol
#include <sol/sol.hpp>

// aengine
#include <aengine/fwd.hpp>

// ecs
#include <easy_ecs/component.hpp>


// General

namespace ecs::util {
    std::list<void(*)(sol::state_view& lua)>& component_definers();
    aengine::StringSet& components_dictionary();


    template<ComponentLike T>
    struct ComponentInitializer {
        static void define(sol::state_view& lua) {
            lua.new_usertype<typename T::value_type>(ecs::component_name<T>());
        }


        ComponentInitializer() {
            component_definers().emplace_back(&define);
            components_dictionary().emplace(ecs::component_name<T>());
        }
    };
}


// Traits

namespace ecs::util {
    template<ers::fixed_string Name>
    struct component_by_name {
        using type = void;
    };

    template<ers::fixed_string Name>
    using component_by_name_t = component_by_name<Name>::type;
}


// Macros

#define AENGINE_ECS_COMPONENT(NAMESPACE, NAME, TYPE) \
    namespace NAMESPACE { using NAME = ::ecs::TComponent<#NAME, TYPE>; } \
    namespace NAMESPACE::internal { inline const ::ecs::util::ComponentInitializer<NAME> NAME##_define_component {}; } \
    template<> struct ::ecs::util::component_by_name<#NAME> { using type = TYPE; }
