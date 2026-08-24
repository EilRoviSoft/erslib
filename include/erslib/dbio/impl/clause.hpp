#pragma once

// std
#include <memory>
#include <ranges>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/traits.hpp>
#include <erslib/dbio/impl/context.hpp>
#include <erslib/dbio/impl/slot.hpp>

// export
#include <erslib/export.hpp>


namespace dbio::impl {
    using Clause = std::polymorphic<class IClause>;

    class ERSLIB_EXPORT IClause {
    public:
        // Member functions

        explicit IClause(SlotRef slot);

        virtual ~IClause() = default;


        // Accessors

        SlotRef slot() const noexcept { return _slot; }


        // Executors

        virtual ers::Status render(Context& ctx) const = 0;


    private:
        SlotRef _slot;
    };


    template<typename T, typename... Args>
    Clause make_clause(Args&&... args) {
        return Clause(std::in_place_type<T>, std::forward<Args>(args)...);
    }
}


namespace dbio::impl {
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
