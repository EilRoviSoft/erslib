#pragma once

// ers
#include <erslib/meta/type_hash.hpp>


namespace ecs {
    template<typename T>
    concept ComponentTag = requires { typename T::value_type; };


    template<typename Tag>
    using component_value_t = typename Tag::value_type;


    template<ComponentTag Tag>
    size_t component_id() {
        return ers::meta::type_hash_v<Tag>;
    }
}
