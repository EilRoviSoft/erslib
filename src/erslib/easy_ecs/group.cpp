#include "erslib/easy_ecs/group.hpp"

// ers
#include <erslib/easy_ecs/registry.hpp>


// Internal

bool ecs::impl::is_valid(const Registry& registry, size_t eid, size_t cid) {
    return registry.has_component(eid, cid);
}

void* ecs::impl::get_component(const Registry& registry, size_t eid, size_t cid) {
    return registry.get_component(eid, cid);
}


// IGroup

void ecs::impl::IGroup::try_add(const Registry& reg, IEntity& entity) {
    if (!has(entity.id()) && is_valid(reg, entity))
        add(reg, entity);
}
