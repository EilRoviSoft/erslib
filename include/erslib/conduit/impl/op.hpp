#pragma once

// std
#include <array>
#include <stdexcept>
#include <string_view>
#include <utility>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/general.hpp>


namespace conduit::impl {
    enum class EOp : u8 {
        Eq, Ne, Lt, Le, Gt, Ge,
        Like, ILike,
    };

    constexpr std::string_view to_string(EOp op) {
        constexpr std::array<std::string_view, 8> table = {
            "=", "<>", "<", "<=", ">", ">=", "LIKE", "ILIKE",
        };

        return table[static_cast<size_t>(op)];
    }

    constexpr EOp to_op(std::string_view s) {
        constexpr std::array<std::pair<std::string_view, EOp>, 8> table = {{
            { "=", EOp::Eq },
            { "<>", EOp::Ne },
            { "<", EOp::Lt },
            { "<=", EOp::Le },
            { ">", EOp::Gt },
            { ">=", EOp::Ge },
            { "LIKE", EOp::Like },
            { "ILIKE", EOp::ILike },
        }};

        for (const auto& [key, value] : table) {
            if (key == s)
                return value;
        }

        throw std::out_of_range("Unknown SQL operator");
    }


    template<ers::fixed_string S>
    constexpr EOp operator""_op() {
        return to_op(S.to_sv());
    }

    constexpr EOp operator""_op(const char* s, size_t n) {
        return to_op({ s, n });
    }
}
