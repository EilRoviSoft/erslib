#pragma once

// std
#include <span>
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/conduit/impl/context.hpp>
#include <erslib/conduit/impl/fwd.hpp>


namespace conduit::impl {
    enum class EArity : bool {
        Single, Multi
    };

    using slot_renderer_t = ers::Status(*)(Context&, std::span<const Clause* const>);


    using SlotRef = const struct Slot*;
    struct Slot {
        std::string_view name;
        EArity arity;
    };


    struct SlotBinding {
        SlotRef slot = nullptr;

        std::string_view prefix;
        std::string_view separator;
        std::string_view suffix;
        std::string_view fallback;

        slot_renderer_t renderer = nullptr;
    };
    using LayoutView = std::span<const SlotBinding>;


    [[nodiscard]]
    constexpr bool same_slot(SlotRef lhs, SlotRef rhs) noexcept {
        return lhs && rhs && lhs->name == rhs->name;
    }
}
