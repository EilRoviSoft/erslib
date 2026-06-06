#include "easy_ecs/entity.hpp"


ecs::Entity::Entity(std::string_view name) :
    name(name) {
}


size_t ecs::Entity::get_id(std::string_view name) {
    return ers::RapidHash<std::string_view> {}(name);
}
size_t ecs::Entity::id() const {
    return get_id(name);
}
