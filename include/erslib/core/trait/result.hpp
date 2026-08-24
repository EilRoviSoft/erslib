#pragma once

// ers
#include <erslib/core/type/result.hpp>


namespace ers::impl {
    template<typename T>
    struct is_result : std::false_type {};

    template<typename T>
    struct is_result<Result<T>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_result_v = is_result<T>::value;

    template<typename T>
    using flattened_result_t = std::conditional_t<is_result_v<T>, T, Result<T>>;
}


namespace ers::impl {
    template<typename T>
        requires is_result_v<T>
    struct result_traits {
        using value_type = T::value_type;
        using error_type = T::error_type;
    };
}


// Exports

namespace ers {
    using impl::is_result;
    using impl::is_result_v;

    using impl::result_traits;
    using impl::flattened_result_t;
}
