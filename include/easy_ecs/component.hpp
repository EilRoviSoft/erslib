#pragma once

// ers
#include <erslib/hashing/rapid.hpp>
#include <erslib/meta/type_name.hpp>

namespace ecs {
    class IComponent {
    public:
        template<typename T>
        static constexpr size_t tid() {
            static constexpr size_t hash = ers::rapidhash<std::string_view>(ers::meta::type_name<T>());
            return hash;
        }

        IComponent() = default;
        virtual ~IComponent() = default;
    };
}
