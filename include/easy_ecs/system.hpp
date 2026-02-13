#pragma once

// std
#include <list>

namespace ecs {
    class ISystem {
    public:
        virtual ~ISystem() = default;
    };
}
