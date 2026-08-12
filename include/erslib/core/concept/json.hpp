#pragma once

// contrib
#include <erslib/contrib/json.hpp>

// ers
#include <erslib/core/trait/variant.hpp>


namespace ers::impl {
    template<typename T>
    concept JsonCompatible = variant_traits<utl::Json::variant_type>::contains_v<T> || utl::internal::is_json_convertible_v<T>;
}
