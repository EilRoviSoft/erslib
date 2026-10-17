#pragma once

// std
#include <span>
#include <string>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/conduit/impl/migration.hpp>
#include <erslib/conduit/impl/migrator.hpp>
#include <erslib/core/filesystem.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


// Service class

namespace conduit::impl {
    class PqxxMigrationStore final : public IMigrationLedger, public IMigrationDatabase {
    public:
        // Member functions

        explicit PqxxMigrationStore(pqxx::work& tx);


        // Accessors

        bool tracks(std::string_view schema) const override;

        bool has_table(std::string_view qualified) const override;


        // Generators

        ers::StringMap<u64> make_applied(std::string_view schema) const override;
        ers::StringMap<recorded_table_t> make_tables(std::string_view schema) const override;


        // Modifiers

        void prepare();

        void execute(std::string_view sql) override;

        void record(std::string_view schema, const migration_t& what, EMigrationEvent outcome) override;
        void remember(std::string_view schema, std::string_view table, const recorded_table_t& what) override;
        void forget(std::string_view schema, std::string_view table) override;


    private:
        pqxx::work& _tx;
    };
}


// Public free functions for execution

namespace conduit::impl {
    std::string_view ERSLIB_EXPORT service_schema();

    ers::Result<migration_report_t> ERSLIB_EXPORT run_migrations(
        pqxx::connection& conn,
        std::span<const migration_target_t> targets,
        const std::filesystem::path& root,
        const migrate_options_t& options = {}
    );
}
