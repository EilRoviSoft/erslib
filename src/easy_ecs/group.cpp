#include "easy_ecs/group.hpp"


void ecs::IGroup::add_if_valid(Entity& entity) {
    if (!has(entity.id()) && is_valid(entity))
        add(entity);
}
