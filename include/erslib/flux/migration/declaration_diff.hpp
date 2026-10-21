#pragma once

// std
#include <span>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/type/maybe.hpp>
#include <erslib/flux/migration/declaration.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    struct declaration_mismatch_t {
        std::string table;
        std::string recorded;
        ers::maybe<table_declaration_t> declared;
    };


    std::vector<std::string> ERSLIB_EXPORT diff_declarations(std::string_view recorded, std::string_view declared);
    std::string ERSLIB_EXPORT describe_mismatches(std::span<const declaration_mismatch_t> what);
}
