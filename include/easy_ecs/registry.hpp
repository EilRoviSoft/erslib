#pragma once

// std
#include <ranges>

// ers
#include <erslib/exception.hpp>
#include <erslib/hashing/algorithm.hpp>

// aengine
#include <aengine/fwd.hpp>

// ecs
#include <easy_ecs/component.hpp>
#include <easy_ecs/entity.hpp>
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

        template<ComponentLike T, typename... Args>
        size_t add_component(size_t id, Args&&... args) {
            Entity& entity = m_entities.at(id);
            size_t cid = component_id<T>();

            if (entity.linked_components.contains(cid)) {
                throw ers::make_runtime_error("Component {} (id: {}) for entity {} already exists",
                    component_name<T>(), cid, id);
            }


            aengine::Object component(std::in_place_type<T>, std::forward<Args>(args)...);
            size_t key = ers::hashing::combine<ers::RapidHash>(entity.name, cid);

            auto [it, inserted] = m_components.try_emplace(key, std::move(component));
            if (!inserted) {
                throw ers::make_runtime_error("Component {} (hash: {}) for entity {} already exist",
                    component_name<T>(), cid, id);
            }

            entity.linked_components[cid] = &it->second;


            // Rn it's guaranteed that entity with new component isn't registered by groups
            // 'cause components are unique per entity.

            for (auto& group : m_groups | std::views::values)
                group->add_if_valid(entity);


            return key;
        }

        template<typename... Ts>
        size_t add_group() {
            size_t key = TGroup<Ts...>::get_id();
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
        aengine::TrivialMap<Entity> m_entities;
        aengine::TrivialMap<aengine::Object> m_components;
        aengine::TrivialMap<GroupPtr> m_groups;


    private:
        template<typename... Ts>
        auto& _get_group() {
            using concrete_group = TGroup<Ts...>;
            size_t id = concrete_group::get_id();
            auto& group = *m_groups.at(id).get();
            return dynamic_cast<concrete_group&>(group);
        }
    };
}
