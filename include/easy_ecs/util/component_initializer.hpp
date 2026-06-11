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
    struct ComponentTypeInfo {
        size_t id;
        std::string_view name;
    };


    std::list<ComponentTypeInfo>& components_container();
    aengine::TrivialMap<ComponentTypeInfo*>& components_by_id();
    aengine::StringMap<ComponentTypeInfo*>& components_by_name();


    std::list<void(*)(sol::state_view& lua)>& component_definers();


    template<ComponentLike T>
    struct ComponentInitializer {
        static void define(sol::state_view& lua) {
            lua.new_usertype<typename T::value_type>(ecs::component_name<T>());
        }


        ComponentInitializer() {
            component_definers().emplace_back(&define);

            auto& ref = components_container().emplace_back(ComponentTypeInfo {
                .id   = ecs::component_id<T>(),
                .name = ecs::component_name<T>()
            });

            components_by_id().emplace(ref.id, &ref);
            components_by_name().emplace(ref.name, &ref);
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

#define EASY_ECS_COMPONENT(NAMESPACE, NAME, TYPE) \
    namespace NAMESPACE { using NAME = ::ecs::TComponent<#NAME, TYPE>; } \
    namespace NAMESPACE::internal { inline const ::ecs::util::ComponentInitializer<NAME> NAME##_define_component {}; } \
    template<> struct ::ecs::util::component_by_name<#NAME> { using type = TYPE; }
