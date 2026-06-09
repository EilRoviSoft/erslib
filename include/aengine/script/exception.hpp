#pragma once

// std
#include <exception>

// ers
#include <erslib/exception/internal.hpp>


// Exception types

namespace aengine {
    ERS_MAKE_EXCEPTION_TYPE(lua_error, std::runtime_error);
    ERS_MAKE_EXCEPTION_TYPE(lua_package_error, lua_error);
    ERS_MAKE_EXCEPTION_TYPE(lua_stage_error, lua_error);
}
