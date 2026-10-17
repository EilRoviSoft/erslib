#pragma once

// std
#include <ranges>
#include <span>
#include <string_view>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/flux/query/slot.hpp>
#include <erslib/flux/sql/binder.hpp>
#include <erslib/flux/sql/context.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    ers::Status ERSLIB_EXPORT check_identifier(std::string_view name, SlotRef slot);
    ers::Status ERSLIB_EXPORT append_identifier(Context& ctx, std::string_view name, SlotRef slot);


    template<std::ranges::input_range R, typename Fn>
    ers::Status append_joined(Context& ctx, R&& items, std::string_view separator, Fn render_one) {
        bool first = true;

        for (auto&& it : items) {
            if (!first)
                ctx.query += separator;
            first = false;

            if (auto s = render_one(ctx, it); !s)
                return s;
        }

        return ers::ok;
    }

    ers::Status ERSLIB_EXPORT append_binders(Context& ctx, std::span<const binder_t> binders, std::string_view separator = ", ");
}
