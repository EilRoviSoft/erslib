#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/core/fwd.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/flux/migration/declaration.hpp>
#include <erslib/flux/migration/migration.hpp>
#include <erslib/flux/migration/options.hpp>
#include <erslib/flux/migration/ports.hpp>
#include <erslib/flux/migration/report.hpp>

// export
#include <erslib/export.hpp>


namespace flux::impl {
    enum class EDatabaseState : u8 {
        Fresh,
        Legacy,
        Tracked,
    };


    EMigrationEvent ERSLIB_EXPORT decide_outcome(EDatabaseState state, const migration_t& what, u32 baseline);
}


namespace flux::impl {
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
