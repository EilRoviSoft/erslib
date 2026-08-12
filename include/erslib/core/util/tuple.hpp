#pragma once

// std
#include <tuple>


namespace ers::impl::util {
    template<typename... Ts>
    std::tuple<Ts&...> pointers_to_references(const std::tuple<Ts*...>& what) {
        auto&& [...args] = what;
        return { *args... };
    }


    template<typename T, typename... Ts>
    std::tuple<T, Ts...> prepend(T&& value, std::tuple<Ts...>&& rest) {
        auto&& [...args] = rest;
        return { std::forward<T>(value), std::forward<Ts>(args)... };
    }
}


// Exports

namespace ers::util {
    using impl::util::pointers_to_references;
    using impl::util::prepend;
}
