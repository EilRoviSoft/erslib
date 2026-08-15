#pragma once

// std
#include <cstdint>
#include <span>
#include <string>

// ers
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/context.hpp>
#include <erslib/dbio/impl/fwd.hpp>


namespace dbio::impl {
    enum class Arity : bool {
        Single, Multi
    };

    using slot_renderer_t = ers::Status(*)(Context&, std::span<const Clause* const>);


    using SlotRef = const struct Slot*;
    struct Slot {
        std::string_view name;
        Arity arity;

        constexpr uint64_t id() const noexcept {
            return ers::RapidHash<std::string_view> {}(name);
        }
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
