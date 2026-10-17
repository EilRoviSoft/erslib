#pragma once

// std
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>


namespace flux::impl {
    inline constexpr u32 declaration_format = 1;

    struct table_declaration_t {
        std::string_view name;
        std::string_view ddl;
        std::string_view text;
        u64 hash = 0;
    };

    struct recorded_table_t {
        u32 format = 0;
        u64 hash = 0;
        std::string text;
    };
}
