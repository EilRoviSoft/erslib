#pragma once

// ecs
#include <easy_ecs/component.hpp>
#include <easy_ecs/entity.hpp>
#include <easy_ecs/registry.hpp>
#include <easy_ecs/system.hpp>

namespace ecs {
    class Engine {
    public:
        template<typename K, typename T>
            requires std::derived_from<T, IComponent>
        void add(const K& k, T what) {
            auto& ref = m_components.insert(k, std::move(what));
            m_components[T::tid()].emplace_back(ref);
        }

        /*template<typename... Args>
        void select_by_type();*/

    protected:
        Registry<IComponent> m_components;
        ers::SwissMap<size_t, std::list<IComponent&>> m_components_by_type;
    };
}
