#pragma once

// std
#include <string>
#include <string_view>
#include <vector>
#include <span>

// ers
#include <erslib/modkit/mod.hpp>

// export
#include <erslib/export.hpp>


namespace ers::impl::modkit {
    struct stage_order_info_t {
        std::string mod;
        std::string phase;
        size_t index;
    };
}

namespace ers::impl::modkit {
    ERSLIB_EXPORT std::vector<std::string> resolve_mods_order(const ModContainer& mods, std::string_view initial_mod);
    ERSLIB_EXPORT std::vector<stage_order_info_t> resolve_stages_order(
        const ModContainer& mods,
        std::span<const std::string> mods_order,
        std::span<const std::string> phases_order
    );
}


// Exports ---------------------------------------------------------------------

namespace ers::modkit {
    using impl::modkit::stage_order_info_t;
    using impl::modkit::resolve_mods_order;
    using impl::modkit::resolve_stages_order;
}
