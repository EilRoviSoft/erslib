#pragma once

// std
#include <string_view>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/flux/migration/declaration.hpp>
#include <erslib/flux/migration/migration.hpp>
#include <erslib/flux/migration/report.hpp>


namespace flux::impl {
    class IMigrationLedger {
    public:
        // Member functions

        virtual ~IMigrationLedger() = default;


        // Accessors

        virtual bool tracks(std::string_view schema) const = 0;


        // Generators

        virtual ers::StringMap<u64> make_applied(std::string_view schema) const = 0;
        virtual ers::StringMap<recorded_table_t> make_tables(std::string_view schema) const = 0;


        // Modifiers

        virtual void record(std::string_view schema, const migration_t& what, EMigrationEvent outcome) = 0;
        virtual void remember(std::string_view schema, std::string_view table, const recorded_table_t& what) = 0;
        virtual void forget(std::string_view schema, std::string_view table) = 0;
    };

    class IMigrationDatabase {
    public:
        // Member functions

        virtual ~IMigrationDatabase() = default;


        // Accessors

        virtual bool has_table(std::string_view qualified) const = 0;


        // Modifiers

        virtual void execute(std::string_view sql) = 0;
    };
}
