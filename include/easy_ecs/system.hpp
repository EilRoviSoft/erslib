#pragma once

// std
#include <list>

// ecs
#include <easy_ecs/component.hpp>

namespace ecs {
    class ISystem {
    public:
        virtual ~ISystem() = default;

    protected:
        std::list<IComponent*> m_components;
    };
}
