#include "erslib/flux/impl/migrate.hpp"

// std
#include <bit>
#include <format>
#include <ranges>
#include <vector>

// ers
#include <erslib/flux/impl/migration_source.hpp>
#include <erslib/flux/impl/migrator.hpp>
#include <erslib/flux/impl/scope.hpp>


#ifndef ERSLIB_FLUX_SCHEMA
#  define ERSLIB_FLUX_SCHEMA "_flux"
#endif


// Details

namespace {
    constexpr std::string_view schema_name = ERSLIB_FLUX_SCHEMA;
}

namespace {
    constexpr char service_sql_data[] = {
#embed "sql/service.sql"
    };

    constexpr std::string_view service_sql(service_sql_data, std::size(service_sql_data));


    consteval std::string_view find_service_query(std::string_view name) {
        constexpr std::string_view marker = "-- name: ";

        for (auto at = service_sql.find(marker); at != std::string_view::npos; at = service_sql.find(marker, at + 1)) {
            const auto name_begin = at + marker.size();
            const auto line_end = service_sql.find('\n', name_begin);

            if (ers::util::trim(service_sql.substr(name_begin, line_end - name_begin)) != name)
                continue;

            const auto next = service_sql.find(marker, line_end);
            return ers::util::trim(service_sql.substr(line_end + 1, next == std::string_view::npos ? next : next - line_end - 1));
        }

        throw "unknown service query";
    }

    template<ers::fixed_string Name>
    std::string_view service_query() {
        static const std::string sql = std::format(find_service_query(Name.to_sv()), schema_name);
        return sql;
    }
}


namespace {
    ers::Result<std::vector<flux::impl::migration_set_t>> load_sets(
        std::span<const flux::impl::migration_target_t> targets,
        const fs::path& root
    ) {
        std::vector<flux::impl::migration_set_t> out;

        for (const auto& target : targets) {
            if (target.schema == schema_name)
                return ers::make_error("Schema '{}' is reserved for flux's service tables", schema_name);

            auto loaded = flux::impl::load_migrations(root / target.schema);

            if (!loaded)
                return loaded.error();

            out.emplace_back(std::move(*loaded));
        }

        return out;
    }
}


// Service class

flux::impl::PqxxMigrationStore::PqxxMigrationStore(pqxx::work& tx) :
    _tx(tx) {
}


bool flux::impl::PqxxMigrationStore::tracks(std::string_view schema) const {
    return _tx.exec(service_query<"tracks">(), pqxx::params { schema })
        .one_field()
        .as<bool>();
}

bool flux::impl::PqxxMigrationStore::has_table(std::string_view qualified) const {
    return _tx.exec(service_query<"has_table">(), pqxx::params { qualified })
        .one_field()
        .as<bool>();
}


ers::StringMap<u64> flux::impl::PqxxMigrationStore::make_applied(std::string_view schema) const {
    ers::StringMap<u64> out;

    const auto rows = _tx.exec(service_query<"applied">(), pqxx::params { schema });

    for (const auto row : rows)
        out.emplace(row[0].as<std::string>(), std::bit_cast<u64>(row[1].as<i64>()));

    return out;
}

ers::StringMap<flux::impl::recorded_table_t> flux::impl::PqxxMigrationStore::make_tables(std::string_view schema) const {
    ers::StringMap<recorded_table_t> out;

    const auto rows = _tx.exec(service_query<"recorded_tables">(),
        pqxx::params { schema }
    );

    for (const auto row : rows) {
        out.emplace(row[0].as<std::string>(), recorded_table_t {
            .format = row[1].as<u32>(),
            .hash   = std::bit_cast<u64>(row[2].as<i64>()),
            .text   = row[3].as<std::string>(),
        });
    }

    return out;
}


void flux::impl::PqxxMigrationStore::prepare() {
    _tx.exec(service_query<"lock">(), pqxx::params { schema_name });
    _tx.exec(service_query<"create_schema">());
    _tx.exec(service_query<"create_migrations">());
    _tx.exec(service_query<"create_tables">());
}

void flux::impl::PqxxMigrationStore::execute(std::string_view sql) {
    _tx.exec(sql);
}

void flux::impl::PqxxMigrationStore::record(std::string_view schema, const migration_t& what, EMigrationEvent outcome) {
    _tx.exec(service_query<"record_migration">(), pqxx::params {
        schema, what.id, what.version, to_string(outcome), std::bit_cast<i64>(what.checksum)
    });
}

void flux::impl::PqxxMigrationStore::remember(std::string_view schema, std::string_view table, const recorded_table_t& what) {
    _tx.exec(service_query<"remember_table">(), pqxx::params {
        schema, table, what.format, std::bit_cast<i64>(what.hash), what.text
    });
}

void flux::impl::PqxxMigrationStore::forget(std::string_view schema, std::string_view table) {
    _tx.exec(service_query<"forget_table">(), pqxx::params { schema, table });
}


// Public free functions

std::string_view flux::impl::service_schema() {
    return schema_name;
}

ers::Result<flux::impl::migration_report_t> flux::impl::run_migrations(
    pqxx::connection& conn,
    std::span<const migration_target_t> targets,
    const fs::path& root,
    const migrate_options_t& options
) {
    auto sets = load_sets(targets, root);

    if (!sets)
        return sets.error();


    auto result = try_with_tx(conn, [&](pqxx::work& tx) -> ers::Result<migration_report_t> {
        PqxxMigrationStore store(tx);
        store.prepare();

        migration_report_t report;

        for (const auto& [target, set] : std::views::zip(targets, *sets)) {
            auto events = Migrator(store, store, options, target, set).run();

            if (!events)
                return events.error();

            report.append_range(std::move(*events));
        }

        return report;
    }, "Flux/Migrate");


    try {
        pqxx::nontransaction(conn).exec("RESET ALL");
    } catch (...) {
    }


    return result;
}
