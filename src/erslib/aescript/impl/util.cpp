#include "erslib/aescript/impl/util.hpp"

// ers
#include <erslib/aescript/properties/presence.hpp>


namespace {
    aescript::impl::Field field_with_presence(std::string name, bool flag) {
        return aescript::impl::Field(std::move(name)) | aescript::impl::properties::presence(flag);
    }
}


aescript::impl::Field aescript::impl::required_field(std::string name) {
    return field_with_presence(std::move(name), true);
}

aescript::impl::Field aescript::impl::optional_field(std::string name) {
    return field_with_presence(std::move(name), false);
}
