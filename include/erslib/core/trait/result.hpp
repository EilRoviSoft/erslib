#pragma once

// ers
#include <erslib/core/type/result.hpp>


namespace ers {
    template<typename T>
    struct is_result : std::false_type {};

    template<typename T>
    struct is_result<Result<T>> : std::true_type {};

    template<typename T>
    static constexpr bool is_result_v = is_result<T>::value;
}


namespace ers {
    template<typename T>
        requires is_result_v<T>
    struct result_traits {
        using value_type = T::value_type;
        using error_type = T::error_type;
    };
}
