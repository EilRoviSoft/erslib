#pragma once

// contrib
#include <erslib/contrib/json/impl.hpp>

// ers
#include <erslib/core/trait/variant.hpp>


namespace utl::impl {
    template<typename T>
    concept JsonCompatible = ers::variant_traits<Json::variant_type>::contains_v<T>
        || json_detail::ConvertibleSource<T, Json>;
}
