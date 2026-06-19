#pragma once

// aengine
#include <erslib/aengine/fwd.hpp>

// ecs
#include <erslib/easy_ecs/component.hpp>
#include <erslib/easy_ecs/entity.hpp>
#include <erslib/easy_ecs/group.hpp>


namespace ecs {
    class Registry {
    public:
        // Tracking

        size_t track_entity(IEntity& entity);

        void track_component(size_t eid, size_t cid, void* component);

        void finalize_entity_groups(size_t eid);


        // Groups

        template<ComponentTag... Tags>
        size_t add_group() {
            size_t key = TGroup<Tags...>::get_id();
            m_groups.emplace(key, std::make_unique<TGroup<Tags...>>());
            return key;
        }

        template<ComponentTag... Tags>
        GroupView<Tags...> view_group() {
            return GroupView<Tags...> { _get_group<Tags...>() };
        }

        template<ComponentTag... Tags>
        GroupWithEntityIdView<Tags...> view_group_with_entity_id() {
            return GroupWithEntityIdView<Tags...> { _get_group<Tags...>() };
        }


        // Queries

        [[nodiscard]]
        bool has_component(size_t eid, size_t cid) const;

        [[nodiscard]]
        void* get_component(size_t eid, size_t cid) const;


    protected:
        aengine::TrivialMap<IEntity*> m_entities;
        aengine::TrivialMap<void*> m_components;
        aengine::TrivialMap<std::unique_ptr<IGroup>> m_groups;


    private:
        template<ComponentTag... Tags>
        TGroup<Tags...>& _get_group() {
            auto& ptr = m_groups.at(TGroup<Tags...>::get_id());
            return static_cast<TGroup<Tags...>&>(*ptr);
        }
    };
}
