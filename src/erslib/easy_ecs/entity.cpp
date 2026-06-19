#include "erslib/easy_ecs/entity.hpp"

// ers
#include <erslib/core/exception.hpp>

// ecs
#include <erslib/easy_ecs/registry.hpp>


// IEntity

void ecs::IEntity::init(Registry& registry) {
    if (_id) {
        throw ers::make_runtime_error_with_trace("Id for entity '{}' can't be set twice, you probably called 'init' twice",
            _name);
    }

    _id = registry.track_entity(*this);


    track_components(registry);
    registry.finalize_entity_groups(id());
}

ecs::IEntity::IEntity(std::string name) :
    _name(std::move(name)) {
}
