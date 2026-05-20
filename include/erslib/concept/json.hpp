#pragma once

// ers
#include <contrib/json.hpp>
#include <erslib/trait/variant.hpp>


namespace ers {
    template<typename T>
    concept JsonCompatible = variant_traits<utl::Json::variant_type>::contains_v<T> || utl::internal::is_json_convertible_v<T>;
}
