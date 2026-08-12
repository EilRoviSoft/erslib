#pragma once

// std
#include <concepts>
#include <tuple>
#include <type_traits>


namespace ers::impl {
    template<size_t Position, typename... Args>
    using arg_n_t = std::tuple_element_t<0, std::tuple<Args...>>;
}


namespace ers::impl {
    template<typename T, size_t Position, typename... Args>
    concept SameAsIn = std::same_as<std::remove_cvref_t<arg_n_t<Position, Args...>>, T>;


    template<typename T, size_t Position, typename... Args>
    concept ConvertibleToIn = std::convertible_to<std::remove_cvref_t<arg_n_t<Position, Args...>>, T>;
}
