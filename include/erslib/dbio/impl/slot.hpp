#pragma once

// std
#include <cstdint>
#include <string>

// ers
#include <erslib/core/hashing/rapid.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/clause.hpp>


namespace dbio::impl {
    enum class Arity : bool {
        Single, Multi
    };

    using slot_renderer_t = ers::Status(*)(build_context_t&, std::span<const IClause* const>);

    using slot_ref = const struct slot_t*;
    struct slot_t {
        std::string_view name;
        std::string_view prefix;
        std::string_view separator;
        std::string_view suffix;
        std::string_view fallback;

        Arity arity;
        slot_renderer_t renderer = nullptr;

        [[nodiscard]]
        constexpr uint64_t id() const noexcept {
            return ers::RapidHash<std::string_view> {}(name);
        }
    };
}
