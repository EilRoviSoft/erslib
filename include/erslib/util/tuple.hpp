#pragma once

// std
#include <tuple>


namespace ers::util {
    template<typename... Ts>
    std::tuple<Ts&...> pointers_to_references(const std::tuple<Ts*...>& what) {
        auto [...args] = what;
        return { *args... };
    }
}
