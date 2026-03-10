#pragma once

// std
#include <exception>
#include <format>


namespace ers {
    template<typename T, typename... Args>
        requires std::derived_from<T, std::exception>
    T make_exception(std::string_view fmt, Args&&... args) {
        return T(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
    }


    template<typename... Args>
    std::runtime_error make_exception(std::string_view fmt, Args&&... args) {
        return make_exception<std::runtime_error>(fmt, std::forward<Args>(args)...);
    }
}
