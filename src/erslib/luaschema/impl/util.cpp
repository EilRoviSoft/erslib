#include "erslib/luaschema/impl/util.hpp"

// ers
#include <erslib/luaschema/properties/presence.hpp>


namespace {
    ers::luaschema::impl::Field field_with_presence(std::string name, bool flag) {
        return ers::luaschema::impl::Field(std::move(name)) | ers::luaschema::impl::properties::presence(flag);
    }
}


ers::luaschema::impl::Field ers::luaschema::impl::required_field(std::string name) {
    return field_with_presence(std::move(name), true);
}

ers::luaschema::impl::Field ers::luaschema::impl::optional_field(std::string name) {
    return field_with_presence(std::move(name), false);
}
