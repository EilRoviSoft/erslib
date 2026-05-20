#pragma once

// std
#include <type_traits>
#include <variant>


// TODO: make API more versatile

namespace ers {
    template<typename T>
    struct variant_traits;

    template<typename... Ts>
    struct variant_traits<std::variant<Ts...>> {
        static constexpr size_t size = sizeof...(Ts);


        using types_tuple = std::tuple<Ts...>;
        using variant_type = std::variant<Ts...>;


        template<size_t I>
        using type_at = std::tuple_element_t<I, types_tuple>;

        template<typename T>
        static constexpr bool contains_v = (std::is_same_v<T, Ts> || ...);

        template<typename T>
        static constexpr size_t index_of() {
            size_t result = size, i = 0;
            ([&] {
                if (std::is_same_v<T, Ts>) {
                    result = i;
                    return true;
                } else {
                    i++;
                    return false;
                }
            }() || ...);
            return result;
        }
    };
}
