#pragma once

// std
#include <string_view>
#include <vector>

// ers
#include <erslib/flux/expr/expr.hpp>
#include <erslib/flux/sql/identifier.hpp>


namespace flux::impl::nodes {
    constexpr EPrecedence next(EPrecedence precedence) noexcept {
        return precedence == EPrecedence::Primary
            ? precedence
            : static_cast<EPrecedence>(static_cast<u8>(precedence) + 1);
    }

    inline bool is_type_name(std::string_view type) {
        while (type.ends_with("[]"))
            type.remove_suffix(2);

        return is_identifier(type);
    }

    inline ers::Status render_list(Context& ctx, const std::vector<Expr>& items) {
        bool first = true;

        for (const auto& it : items) {
            if (!first)
                ctx.query += ", ";
            first = false;

            ERS_TRY(it.render(ctx));
        }

        return ers::ok;
    }
}
