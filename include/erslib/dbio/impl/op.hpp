#pragma once

// std
#include <array>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>


namespace dbio::impl {
    enum class Op : u8 {
        Eq, Ne, Lt, Le, Gt, Ge,
        Like, ILike
    };

    inline std::string_view op_sql(Op op) {
        static constexpr std::array<std::string_view, 8> text = {
            "=", "<>", "<", "<=", ">", ">=", "LIKE", "ILIKE"
        };

        return text[static_cast<size_t>(op)];
    }
}
