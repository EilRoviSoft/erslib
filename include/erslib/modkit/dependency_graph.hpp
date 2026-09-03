#pragma once

// std
#include <string>
#include <vector>
#include <span>

// ers
#include <erslib/modkit/mod.hpp>


namespace ers::modkit::impl {
    struct stage_order_info_t {
        std::string mod;
        std::string phase;
        size_t index;
    };
}

namespace ers::modkit::impl {
    std::vector<std::string> resolve_mods_order(const ModContainer& mods, std::string_view initial_mod);
    std::vector<stage_order_info_t> resolve_stages_order(
        const ModContainer& mods,
        std::span<const std::string> mods_order,
        std::span<const std::string> phases_order
    );
}


// Exports

namespace ers::modkit {
    using impl::stage_order_info_t;
    using impl::resolve_mods_order;
    using impl::resolve_stages_order;
}
