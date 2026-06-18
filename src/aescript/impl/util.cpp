#include "aescript/impl/util.hpp"

// aescript
#include <aescript/verifiers/presence.hpp>


namespace {
    aescript::Field field_with_presence(std::string name, bool flag) {
        return aescript::Field(std::move(name)) | aescript::properties::presence(flag);
    }
}


aescript::Field aescript::required_field(std::string name) {
    return field_with_presence(std::move(name), true);
}

aescript::Field aescript::optional_field(std::string name) {
    return field_with_presence(std::move(name), false);
}
