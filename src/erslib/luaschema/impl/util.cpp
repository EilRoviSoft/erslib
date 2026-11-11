#include "erslib/luaschema/impl/util.hpp"

// std
#include <utility>

// ers
#include <erslib/luaschema/properties/presence.hpp>


namespace {
    ers::impl::luaschema::Field field_with_presence(std::string name, bool flag) {
        return ers::impl::luaschema::Field(std::move(name)) | ers::impl::luaschema::properties::presence(flag);
    }
}


ers::impl::luaschema::Field ers::impl::luaschema::required_field(std::string name) {
    return field_with_presence(std::move(name), true);
}

ers::impl::luaschema::Field ers::impl::luaschema::optional_field(std::string name) {
    return field_with_presence(std::move(name), false);
}
