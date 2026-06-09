#pragma once

// std
#include <format>

// sol
#include <sol/error.hpp>
#include <sol/state_view.hpp>


// Exception types

namespace aengine {
    ERS_MAKE_EXCEPTION_TYPE_WITH_BASE(lua_error, std::runtime_error);
    ERS_MAKE_EXCEPTION_TYPE_WITH_ERS_BASE(lua_package_error, lua_error);
    ERS_MAKE_EXCEPTION_TYPE_WITH_ERS_BASE(lua_stage_error, lua_error);
}


// Utility

namespace aengine {
    template<typename Fn, typename... Args>
    auto make_lua_error(sol::state_view& lua, std::format_string<Args...> fmt, Args&&... args) {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        luaL_traceback(lua, lua, message.c_str(), 1);

        std::string trace = lua_tostring(lua, -1);
        lua_pop(lua, 1);

        return std::make_exception_ptr(Fn {}(1, std::move(trace)));
    }
}
