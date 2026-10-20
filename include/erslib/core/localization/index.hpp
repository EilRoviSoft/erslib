#pragma once

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/localization/fwd.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    struct locale_index_t {
        locale_config_ptr config;
        StringMap<DomainPtr> domains;
    };
}
