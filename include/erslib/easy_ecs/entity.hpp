#pragma once

// std
#include <format>
#include <string>

// ers
#include <erslib/core/meta/type_name.hpp>
#include <erslib/core/pattern/counter.hpp>

// ecs
#include <erslib/easy_ecs/fwd.hpp>


namespace ecs {
    template<typename T>
    std::string make_canonical_name() {
        return std::format("{}:{}", ers::meta::type_name_v<T>, ers::pattern::tagged_counter<T>());
    }


    class IEntity {
        friend Registry;

    public:
        // Member functions

        virtual ~IEntity() = default;


        // Accessors

        [[nodiscard]]
        // id is set only after 'init' is called
        size_t id() const { return _id; }

        [[nodiscard]]
        std::string_view name() const { return _name; }


        // Modifiers

        void init(Registry& registry);


    protected:
        explicit IEntity(std::string name);


        virtual void track_components(Registry& registry) = 0;


    private:
        std::string _name;
        size_t _id = 0;
    };
}
