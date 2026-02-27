#pragma once

// ers
#include <erslib/hashing/rapid.hpp>

// ecs
#include <easy_ecs/fwd.hpp>


namespace ecs {
    class Entity {
    public:
        std::string name;
        TrivialMap<Object*> linked_components;


        // Constructors

        explicit Entity(std::string_view name) :
            name(name) {
        }


        // Properties

        static size_t get_id(std::string_view name) { return ers::RapidHash<std::string_view> {}(name); }
        size_t id() const { return get_id(name); }
    };
}
