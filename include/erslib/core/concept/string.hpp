#pragma once

// std
#include <string>
#include <string_view>
#include <type_traits>


namespace ers::impl {
    template<typename T>
    concept StringConvertible =
        std::is_convertible_v<T, std::string_view>
        || std::is_convertible_v<T, std::string>
        || std::is_same_v<std::remove_reference_t<T>, const char*>;
}
