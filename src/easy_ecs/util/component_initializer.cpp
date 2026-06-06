#include "easy_ecs/util/component_initializer.hpp"


std::list<void(*)(sol::state_view& lua)>& ecs::util::component_definers() {
    static std::list<void(*)(sol::state_view& lua)> catalog;
    return catalog;
}
aengine::StringSet& ecs::util::components_dictionary() {
    static aengine::StringSet dictionary;
    return dictionary;
}
