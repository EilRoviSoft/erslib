#pragma once

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/localization/fwd.hpp>


// Definition ------------------------------------------------------------------

namespace ers::impl {
    // What readers see at once: folders are always built for this very config.
    struct locale_index_t {
        locale_config_ptr config;
        StringMap<FolderPtr> folders;
    };
}
