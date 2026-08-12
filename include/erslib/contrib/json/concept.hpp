#pragma once

// contrib
#include <erslib/contrib/json/impl.hpp>

// ers
#include <erslib/core/trait/variant.hpp>


namespace utl::impl {
    template<typename T>
    concept JsonCompatible = ers::variant_traits<Node::variant_type>::contains_v<T> || is_json_convertible_v<T>;
}
