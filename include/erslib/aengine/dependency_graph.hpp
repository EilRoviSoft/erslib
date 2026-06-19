#pragma once

// std
#include <string>
#include <vector>
#include <span>

// aengine
#include <erslib/aengine/mod.hpp>


namespace aengine::internal {
    struct stage_order_info_t {
        std::string mod;
        std::string phase;
        size_t index;
    };
}


namespace aengine {
    std::vector<std::string> resolve_mods_order(const ModContainer& mods, std::string_view initial_mod);
    std::vector<internal::stage_order_info_t> resolve_stages_order(
        const ModContainer& mods,
        std::span<const std::string> mods_order,
        std::span<const std::string> phases_order
    );
}
