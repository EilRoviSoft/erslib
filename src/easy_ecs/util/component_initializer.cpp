#include "easy_ecs/util/component_initializer.hpp"


std::list<ecs::util::ComponentTypeInfo>& ecs::util::components_container() {
    static std::list<ComponentTypeInfo> container;
    return container;
}

aengine::TrivialMap<ecs::util::ComponentTypeInfo*>& ecs::util::components_by_id() {
    static aengine::TrivialMap<ComponentTypeInfo*> by_id;
    return by_id;
}

aengine::StringMap<ecs::util::ComponentTypeInfo*>& ecs::util::components_by_name() {
    static aengine::StringMap<ComponentTypeInfo*> by_name;
    return by_name;
}


std::list<void(*)(sol::state_view& lua)>& ecs::util::component_definers() {
    static std::list<void(*)(sol::state_view& lua)> catalog;
    return catalog;
}
