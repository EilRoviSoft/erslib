#pragma once

// std
#include <cstdint>
#include <span>
#include <string>

// ers
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/context.hpp>


namespace dbio::impl {
    class IClause;

    enum class Arity : bool {
        Single, Multi
    };

    using slot_renderer_t = ers::Status(*)(Context&, std::span<const IClause* const>);

    using SlotRef = const struct Slot*;
    using SlotView = std::span<const SlotRef>;

    struct Slot {
        std::string_view name;
        std::string_view prefix;
        std::string_view separator;
        std::string_view suffix;
        std::string_view fallback;

        Arity arity;
        slot_renderer_t renderer = nullptr;

        constexpr uint64_t id() const noexcept {
            return ers::RapidHash<std::string_view> {}(name);
        }
    };


    [[nodiscard]]
    constexpr bool same_slot(SlotRef lhs, SlotRef rhs) noexcept {
        return lhs && rhs && lhs->name == rhs->name;
    }
}
