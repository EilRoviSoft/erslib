#pragma once


// Constant

#include <erslib/flux/impl/constant.hpp>

namespace flux {
    using impl::undefined_id;
}


// Database

#include <erslib/flux/impl/database.hpp>

namespace flux {
    using impl::Database;
}


// Generator

#include <erslib/flux/impl/generator.hpp>

namespace flux {
    using impl::ReadableRow;

    using impl::row_reader;
    using impl::RowGenerator;
}

namespace flux {
    using impl::for_each_row;
    using impl::read_one;
    using impl::read_at_most_one;
}


// Options

#include <erslib/flux/impl/options.hpp>

namespace flux {
    using impl::db_options_t;
    using impl::pool_options_t;
}


// QueryStore

#include <erslib/flux/impl/query_store.hpp>

namespace flux {
    using impl::QueryStore;
    using impl::queries;
}


// Scope

#include <erslib/flux/impl/scope.hpp>

namespace flux {
    using impl::ConnectionSource;

    using impl::with_conn;
    using impl::with_tx;
    using impl::with_query;

    using impl::TryQueryResult;

    using impl::try_invoke;
    using impl::try_with_conn;
    using impl::try_with_tx;
    using impl::try_with_query;
}


// Migrations

#include <erslib/flux/impl/migrate.hpp>

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
