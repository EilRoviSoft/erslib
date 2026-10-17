#pragma once

// std
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/type/general.hpp>
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/result.hpp>


// Details

namespace flux::impl {
    inline constexpr u32 declaration_format = 1;


    enum class EApplyOn : u8 {
        Existing,
        Fresh,
        Always,
        Repeatable,
    };

    enum class EDatabaseState : u8 {
        Fresh,
        Legacy,
        Tracked,
    };

    enum class EMigrationEvent : u8 {
        Applied,
        Stamped,
        Skipped,
        Edited,
        Adopted,
        Accepted,
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

    struct migration_event_t {
        EMigrationEvent kind;
        std::string subject;

        bool operator==(const migration_event_t&) const = default;
    };
    using migration_report_t = std::vector<migration_event_t>;
}

namespace flux::impl {
    constexpr std::string_view to_string(EMigrationEvent what) {
        switch (what) {
            case EMigrationEvent::Applied:
                return "applied";

            case EMigrationEvent::Stamped:
                return "stamped";

            case EMigrationEvent::Skipped:
                return "skipped";

            case EMigrationEvent::Edited:
                return "edited after being applied";

            case EMigrationEvent::Adopted:
                return "adopted";

            case EMigrationEvent::Accepted:
                return "declaration accepted";
        }

        return {};
    }
}


// Free functions

namespace flux::impl {
    ers::Result<migration_t> parse_migration(std::string_view filename, std::string sql);
}
