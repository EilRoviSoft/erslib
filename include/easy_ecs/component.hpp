#pragma once

// ers
#include <erslib/hashing/rapid.hpp>
#include <erslib/type/fixed_string.hpp>


// Definition

namespace ecs {
    template<ers::fixed_string Name, typename T>
    struct TComponent {
        // Aliases

        using value_type = T;
        static constexpr auto name = Name;


        // Data

        T data;


        // Constructors

        TComponent() = default;

        template<typename... Args>
            requires std::constructible_from<T, Args...>
        explicit TComponent(Args&&... args) :
            data(std::forward<Args>(args)...) {
        }
    };


    template<typename T>
    constexpr size_t component_id() {
        return ers::RapidHash<ers::fixed_string<T::name.size() + 1>> {}(T::name);
    }

    template<typename T>
    constexpr std::string_view component_name() {
        return { T::name.value, T::name.size() };
    }
}


// Utility

namespace ecs {
    template<typename T>
    concept ComponentLike = requires {
        typename T::value_type;
        { T::name };
    };
}
