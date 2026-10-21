#pragma once

// std
#include <exception>
#include <format>
#include <string>
#include <utility>

// sol
#include <sol/error.hpp>
#include <sol/state_view.hpp>


// Implementation --------------------------------------------------------------

namespace ers::luaschema::impl {
    template<typename T, typename... Args>
    auto tunnel_error(sol::state_view& lua, std::format_string<Args...> fmt, Args&&... args) {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        luaL_traceback(lua, lua, message.c_str(), 1);
        return std::make_exception_ptr(T(std::move(message)));
    }
}


// Exports ---------------------------------------------------------------------

namespace ers::luaschema {
    using impl::tunnel_error;
}
