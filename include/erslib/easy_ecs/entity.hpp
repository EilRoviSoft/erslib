#pragma once

// std
#include <format>
#include <string>

// ers
#include <erslib/core/meta.hpp>
#include <erslib/core/pattern.hpp>
#include <erslib/easy_ecs/fwd.hpp>


namespace ecs::impl {
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


// Exports

namespace ecs {
    using impl::make_canonical_name;
    using impl::IEntity;
}
