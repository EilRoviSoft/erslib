#pragma once

// std
#include <exception>
#include <stdexcept>

// ers
#include <erslib/core/exception.hpp>


// Implementation --------------------------------------------------------------

namespace ers::impl::luaschema {
    ERS_MAKE_EXCEPTION_TYPE(lua_error, std::runtime_error);
    ERS_MAKE_EXCEPTION_TYPE(lua_package_error, lua_error);
    ERS_MAKE_EXCEPTION_TYPE(lua_stage_error, lua_error);
}


// Exports ---------------------------------------------------------------------

namespace ers::luaschema {
    using impl::luaschema::lua_error;
    using impl::luaschema::lua_package_error;
    using impl::luaschema::lua_stage_error;

    ERS_MAKE_EXCEPTION_EXPORTS(impl, lua_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, lua_package_error);
    ERS_MAKE_EXCEPTION_EXPORTS(impl, lua_stage_error);
}
