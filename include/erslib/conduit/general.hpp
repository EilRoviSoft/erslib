#pragma once


// Constant

#include <erslib/conduit/impl/constant.hpp>

namespace conduit {
    using impl::undefined_id;
}


// Database

#include <erslib/conduit/impl/database.hpp>

namespace conduit {
    using impl::Database;
}


// Generator

#include <erslib/conduit/impl/generator.hpp>

namespace conduit {
    using impl::ReadableRow;

    using impl::row_reader;
    using impl::RowGenerator;
}

namespace conduit {
    using impl::for_each_row;
    using impl::read_one;
    using impl::read_at_most_one;
}


// Options

#include <erslib/conduit/impl/options.hpp>

namespace conduit {
    using impl::db_options_t;
    using impl::pool_options_t;
}


// QueryStore

#include <erslib/conduit/impl/query_store.hpp>

namespace conduit {
    using impl::QueryStore;
    using impl::queries;
}


// Scope

#include <erslib/conduit/impl/scope.hpp>

namespace conduit {
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

#include <erslib/conduit/impl/migrate.hpp>

namespace conduit {
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
