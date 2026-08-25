#pragma once

// std
#include <array>
#include <string_view>

// frozen
#include <frozen/unordered_map.h>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/general.hpp>


namespace dbio::impl {
    enum class Op : u8 {
        Eq, Ne, Lt, Le, Gt, Ge,
        Like, ILike,
    };

    constexpr std::string_view to_string(Op op) {
        static frozen::unordered_map<Op, std::string_view, 8> conversion_table = {
            { Op::Eq, "=" },
            { Op::Ne, "<>" },
            { Op::Lt, "<" },
            { Op::Le, "<=" },
            { Op::Gt, ">" },
            { Op::Ge, ">=" },
            { Op::Like, "LIKE" },
            { Op::ILike, "ILIKE" },
        };

        return conversion_table.at(static_cast<size_t>(op));
    }

    constexpr Op to_op(std::string_view s) {
        static frozen::unordered_map<std::string_view, Op, 8> conversion_table = {
            { "=", Op::Eq },
            { "<>", Op::Ne },
            { "<", Op::Lt },
            { "<=", Op::Le },
            { ">", Op::Gt },
            { ">=", Op::Ge },
            { "LIKE", Op::Like },
            { "ILIKE", Op::ILike },
        };

        return conversion_table.at(s);
    }


    template<ers::fixed_string S>
    constexpr Op operator""_op() {
        return to_op(S.to_sv());
    }

    constexpr Op operator""_op(const char* s, size_t n) {
        return to_op({ s, n });
    }
}
