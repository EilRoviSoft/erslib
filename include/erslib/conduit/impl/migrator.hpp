#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/conduit/impl/migration.hpp>
#include <erslib/core/fwd.hpp>
#include <erslib/core/type/result.hpp>

// export
#include <erslib/export.hpp>


namespace conduit::impl {
    EMigrationEvent ERSLIB_EXPORT decide_outcome(EDatabaseState state, const migration_t& what, u32 baseline);
}


namespace conduit::impl {
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


namespace conduit::impl {
    class ERSLIB_EXPORT Migrator {
    public:
        // Member functions

        Migrator(
            IMigrationLedger& ledger,
            IMigrationDatabase& database,
            const migrate_options_t& options,
            const migration_target_t& target,
            const migration_set_t& set
        );


        // Executors

        ers::Result<migration_report_t> run() &&;


    private:
        IMigrationLedger& _ledger;
        IMigrationDatabase& _database;
        const migrate_options_t& _options;
        const migration_target_t& _target;
        const migration_set_t& _set;

        EDatabaseState _state = EDatabaseState::Fresh;
        ers::StringMap<u64> _applied;
        ers::StringMap<recorded_table_t> _recorded;
        migration_report_t _report;


        // Accessors

        const table_declaration_t* _get_declared(std::string_view table) const;
        u32 _get_baseline() const;


        // Generators

        std::vector<std::string> _make_orphans() const;
        std::string _make_subject(const migration_t& migration) const;


        // Validators

        EDatabaseState _detect() const;
        ers::Status _validate_directives() const;


        // Modifiers

        ers::Status _execute(std::string_view what, std::string_view sql);

        ers::Status _apply_versioned();
        void _apply_directives(const migration_t& migration);
        ers::Status _apply_repeatable();

        ers::Status _bootstrap();
        ers::Status _reconcile_declarations();

        void _record(const migration_t& migration, EMigrationEvent outcome);
        void _remember(std::string_view table, recorded_table_t what);
        void _remember(const table_declaration_t& table);
        void _forget(std::string_view table);
        bool _try_accept(std::string_view table);
        void _add_report(EMigrationEvent kind, std::string subject);
    };
}
