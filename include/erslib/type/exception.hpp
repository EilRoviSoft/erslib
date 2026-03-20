#pragma once

// std
#include <exception>
#include <format>


namespace ers {
    template<typename T, typename... Args>
        requires std::derived_from<T, std::exception>
    T make_exception(const std::format_string<Args...>& fmt, Args&&... args) {
        return T(std::format(fmt, std::forward<Args>(args)...));
    }


    template<typename... Args>
    std::runtime_error make_exception(const std::format_string<Args...>& fmt, Args&&... args) {
        return make_exception<std::runtime_error>(fmt, std::forward<Args>(args)...);
    }
}
