#pragma once

// std
#include <string>

// aengine
#include <aengine/fwd.hpp>


namespace ecs {
    class Entity {
    public:
        std::string name;
        aengine::TrivialMap<aengine::Object*> linked_components;


        // Constructors

        explicit Entity(std::string_view name);


        // Properties

        static size_t get_id(std::string_view name);
        size_t id() const;
    };
}
