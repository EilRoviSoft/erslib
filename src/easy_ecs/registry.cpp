#include "easy_ecs/registry.hpp"

// std
#include <ranges>

// ers
#include <erslib/exception/runtime_error.hpp>
#include <erslib/hashing/algorithm.hpp>
#include <erslib/hashing/rapid.hpp>


// Tracking

size_t ecs::Registry::track_entity(IEntity& entity) {
    size_t id = ers::RapidHash<std::string_view> {}(entity.name());
    m_entities.emplace(id, &entity);
    return id;
}

void ecs::Registry::track_component(size_t eid, size_t cid, void* component) {
    size_t key = ers::hashing::combine<ers::RapidHash>(eid, cid);

    auto [it, inserted] = m_components.try_emplace(key, component);
    if (!inserted) {
        throw ers::make_runtime_error("Component (id: {}) already exist. It's either key collision, or entity (id: {}) already has this component.",
            cid, eid);
    }
}

void ecs::Registry::finalize_entity_groups(size_t eid) {
    auto& entity = m_entities.at(eid);

    for (auto& group : m_groups | std::views::values)
        group->try_add(*this, *entity);
}


// Queries

bool ecs::Registry::has_component(size_t eid, size_t cid) const {
    return m_components.contains(ers::hashing::combine<ers::RapidHash>(eid, cid));
}

void* ecs::Registry::get_component(size_t eid, size_t cid) const {
    return m_components.at(ers::hashing::combine<ers::RapidHash>(eid, cid));
}
