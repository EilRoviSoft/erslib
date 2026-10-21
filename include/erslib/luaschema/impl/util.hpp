#pragma once

// std
#include <string>

// ers
#include <erslib/luaschema/impl/field.hpp>

// export
#include <erslib/export.hpp>


namespace ers::luaschema::impl {
    ERSLIB_EXPORT Field required_field(std::string name);
    ERSLIB_EXPORT Field optional_field(std::string name);
}
