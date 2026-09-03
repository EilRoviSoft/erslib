#pragma once

// ers
#include <erslib/luaschema/impl/field.hpp>


namespace ers::luaschema::impl {
    Field required_field(std::string name);
    Field optional_field(std::string name);
}
