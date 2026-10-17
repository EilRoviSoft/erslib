#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/optional.hpp>


namespace flux::impl {
    enum class EApplyOn : u8 {
        Existing,
        Fresh,
        Always,
        Repeatable,
    };

    struct table_claim_t {
        std::string table;
        u64 hash = 0;
    };

    struct migration_t {
        std::string id;
        ers::optional<u32> version;
        EApplyOn on = EApplyOn::Existing;
        std::string sql;
        u64 checksum = 0;
        std::vector<table_claim_t> claims;
        std::vector<std::string> drops;
    };

    struct migration_set_t {
        std::vector<migration_t> versioned;
        std::vector<migration_t> repeatable;
    };
}
