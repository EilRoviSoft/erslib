#pragma once

// std
#include <exception>

// ers
#include <erslib/core/exception/internal.hpp>


// Exception types

namespace aescript::impl {
    ERS_MAKE_EXCEPTION_TYPE(lua_error, std::runtime_error);
    ERS_MAKE_EXCEPTION_TYPE(lua_package_error, lua_error);
    ERS_MAKE_EXCEPTION_TYPE(lua_stage_error, lua_error);
}


// Exports

namespace aescript {
    using impl::lua_error;
    using impl::lua_package_error;
    using impl::lua_stage_error;

    using impl::make_lua_error;
    using impl::make_lua_error_with_trace;
    using impl::make_lua_package_error;
    using impl::make_lua_package_error_with_trace;
    using impl::make_lua_stage_error;
    using impl::make_lua_stage_error_with_trace;
}
