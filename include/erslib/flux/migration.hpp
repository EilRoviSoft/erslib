#pragma once

// Includes --------------------------------------------------------------------

#include <erslib/flux/migration/declaration.hpp>
#include <erslib/flux/migration/migrate.hpp>
#include <erslib/flux/migration/migration.hpp>
#include <erslib/flux/migration/options.hpp>
#include <erslib/flux/migration/report.hpp>


// Exports ---------------------------------------------------------------------

namespace flux {
    using impl::EApplyOn;
    using impl::EMigrationEvent;
    using impl::migrate_options_t;
    using impl::migration_event_t;
    using impl::migration_report_t;
    using impl::migration_target_t;
    using impl::table_declaration_t;
    using impl::service_schema;
    using impl::run_migrations;
}
