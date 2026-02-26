#pragma once

// std
#include <format>
#include <ranges>
#include <stdexcept>

// ers
#include <erslib/hashing/algorithm.hpp>
#include <erslib/meta/type_info.hpp>

// ecs
#include <easy_ecs/entity.hpp>
#include <easy_ecs/fwd.hpp>
#include <easy_ecs/group.hpp>

namespace ecs {
    class Registry {
    public:
        // Modifiers

        size_t add_entity(std::string_view name) {
            size_t key = Entity::get_id(name);
            m_entities.emplace(key, Entity(name));
            return key;
        }

        template<typename T, typename... Args>
        size_t add_component(size_t id, Args&&... args) {
            Entity& entity = m_entities.at(id);

            Object component(std::in_place_type<T>, std::forward<Args>(args)...);

            size_t type_hash = component.type();
            size_t key = ers::hashing::combine<ers::RapidHash>(entity.name, type_hash);

            auto [it, inserted] = m_components.try_emplace(key, std::move(component));
            if (!inserted)
                throw std::runtime_error(
                    std::format(
                        "Component {} (hash: {}) for entity {} already exist",
                        ers::meta::type_name_v<T>, ers::meta::type_hash_v<T>, id
                    )
                );

            entity.linked_components[type_hash] = &it->second;

            // Rn it's guaranteed that entity with new component isn't registered by groups
            // 'cause components are unique per entity.

            for (auto& group : m_groups | std::views::values)
                group->add_if_valid(entity);

            return key;
        }

        template<typename... Ts>
        size_t add_group() {
            size_t key = Group<Ts...>::get_id();
            m_groups.emplace(key, make_group<Ts...>());
            return key;
        }


        // Observers

        template<typename... Ts>
        GroupView<Ts...> view_group() {
            return { _get_group<Ts...>() };
        }
        template<typename... Ts>
        GroupWithEntityIdView<Ts...> view_group_with_entity_id() {
            return { _get_group<Ts...>() };
        }


    protected:
        TrivialMap<Entity> m_entities;
        TrivialMap<Object> m_components;
        TrivialMap<GroupPtr> m_groups;


    private:
        template<typename... Ts>
        auto& _get_group() {
            using concrete_group = Group<Ts...>;
            size_t id = concrete_group::get_id();
            auto& group = *m_groups.at(id).get();
            return dynamic_cast<concrete_group&>(group);
        }
    };
}
