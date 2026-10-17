#pragma once

// std
#include <map>
#include <span>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/flux/migration/declaration.hpp>


namespace flux::impl {
    struct migration_target_t {
        std::string_view schema;
        std::string_view create_schema;
        std::span<const table_declaration_t> tables;
    };

    struct migrate_options_t {
        std::map<std::string, u32, std::less<>> baseline;
        bool adopt = false;
        bool accept_declarations = false;
    };
}
