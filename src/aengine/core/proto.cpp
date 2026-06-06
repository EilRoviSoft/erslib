#include "aengine/core/proto.hpp"


// field_property_t

aengine::field_property_t aengine::field_property_t::required(bool value) {
    return {
        .type        = Required,
        .is_required = value
    };
}
