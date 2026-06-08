#pragma once

// std
#include <string_view>
#include <vector>
#include <span>

// aengine
#include <aengine/core/mod.hpp>
#include <aengine/enum/phase.hpp>

#include "easy_ecs/util/component_initializer.hpp"
#include "easy_ecs/util/component_initializer.hpp"


namespace aengine::internal {
    struct stage_info_t {
        std::string mod;
        Phase phase;
        size_t index;
    };
}


namespace aengine {
    std::vector<std::string> resolve_mods_order(const ModContainer& mods, std::string_view initial_mod);
    std::vector<internal::stage_info_t> resolve_stages_order(const ModContainer& mods, std::span<const std::string> mods_order);
}
