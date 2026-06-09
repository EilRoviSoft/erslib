#pragma once

// std
#include <format>

// sol
#include <sol/error.hpp>
#include <sol/state_view.hpp>


// Utility

namespace aengine::script {
    template<typename T, typename... Args>
    auto tunnel_error(sol::state_view& lua, std::format_string<Args...> fmt, Args&&... args) {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        luaL_traceback(lua, lua, message.c_str(), 1);
        return std::make_exception_ptr(T(std::move(message)));
    }
}
