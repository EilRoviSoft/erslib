// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <algorithm>
#include <format>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/core/hashing/fnv.hpp>
#include <erslib/flux/migration/declaration_diff.hpp>
#include <erslib/flux/migration/migrator.hpp>
#include <erslib/flux/migration/source.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::impl;
using namespace std::string_view_literals;


// Details ---------------------------------------------------------------------

namespace {
    migration_t parsed(std::string_view filename, std::string sql = "SELECT 1;") {
        auto result = parse_migration(filename, std::move(sql));
        REQUIRE(result);
        return std::move(*result);
    }

    bool rejected(std::string_view filename, std::string sql = "SELECT 1;") {
        return !parse_migration(filename, std::move(sql));
    }

    std::string claim(std::string_view table, std::string_view text) {
        return std::format("-- flux: table {} {:016x}\n", table, ers::Fnv1AHash<std::string_view> {}(text));
    }


    class FakeStore final : public IMigrationLedger, public IMigrationDatabase {
    public:
        bool tracks(std::string_view) const override { return !recorded.empty(); }
        ers::StringMap<u64> make_applied(std::string_view) const override { return migrations; }
        ers::StringMap<recorded_table_t> make_tables(std::string_view) const override { return recorded; }

        void record(std::string_view, const migration_t& what, EMigrationEvent) override {
            migrations.insert_or_assign(what.id, what.checksum);
        }

        void remember(std::string_view, std::string_view table, const recorded_table_t& what) override {
            recorded.insert_or_assign(std::string(table), what);
        }

        void forget(std::string_view, std::string_view table) override {
            recorded.erase(std::string(table));
        }

        bool has_table(std::string_view qualified) const override {
            return std::ranges::contains(existing_tables, qualified);
        }

        void execute(std::string_view sql) override {
            if (sql == failing_sql)
                throw std::runtime_error("division by zero");

            executed.emplace_back(sql);
        }


        std::vector<std::string> existing_tables;
        std::vector<std::string> executed;
        ers::StringMap<u64> migrations;
        ers::StringMap<recorded_table_t> recorded;
        std::string failing_sql;
    };


    constexpr std::string_view sales_v1 = "id INTEGER NOT NULL";
    constexpr std::string_view sales_v2 = "banner TEXT NOT NULL\nid INTEGER NOT NULL";

    constexpr table_declaration_t sales {
        .name = "public.sales",
        .ddl  = "CREATE TABLE IF NOT EXISTS public.sales (...)",
        .text = sales_v2,
        .hash = ers::Fnv1AHash<std::string_view> {}(sales_v2),
    };


    struct Scenario {
        FakeStore store;
        migration_set_t set;
        migrate_options_t options;

        ers::Result<migration_report_t> run() {
            constexpr migration_target_t target {
                .schema        = "public",
                .create_schema = "CREATE SCHEMA IF NOT EXISTS public",
                .tables        = std::span(&sales, 1),
            };

            return Migrator(store, store, options, target, set).run();
        }

        void track(std::string_view text) {
            store.existing_tables.emplace_back(sales.name);
            store.recorded.insert_or_assign(std::string(sales.name), recorded_table_t {
                .format = declaration_format,
                .hash   = ers::Fnv1AHash<std::string_view> {}(text),
                .text   = std::string(text),
            });
        }
    };

    bool reported(const migration_report_t& report, EMigrationEvent kind, std::string_view subject) {
        return std::ranges::contains(report, migration_event_t { .kind = kind, .subject = std::string(subject) });
    }
}


// Source ----------------------------------------------------------------------

ERS_TEST_CASE("fnv1a", "[flux][compile_time]") {
    static_assert(ers::Fnv1AHash<std::string_view> {}("") == 0xcbf29ce484222325ull);
    static_assert(ers::Fnv1AHash<std::string_view> {}("a") == 0xaf63dc4c8601ec8cull);
    static_assert(ers::Fnv1AHash<std::string_view> {}("foobar") == 0x85944171f73967e8ull);
}

ERS_TEST_CASE("checksum", "[flux]") {
    CHECK(parsed("0001_x.always.sql", "SELECT 1;\r\nSELECT 2;").checksum
        == parsed("0001_x.always.sql", "SELECT 1;\nSELECT 2;").checksum);
}

ERS_TEST_CASE("file names", "[flux]") {
    const auto existing = parsed("0001_products_add_sku.existing.sql");

    CHECK(existing.id == "0001_products_add_sku");
    CHECK(existing.version == 1u);
    CHECK(existing.on == EApplyOn::Existing);

    CHECK(parsed("0002_seed.fresh.sql").on == EApplyOn::Fresh);
    CHECK(parsed("0003_fix.always.sql").on == EApplyOn::Always);

    const auto repeatable = parsed("best_sale.repeatable.sql");

    CHECK(repeatable.id == "best_sale");
    CHECK_FALSE(repeatable.version);
    CHECK(repeatable.on == EApplyOn::Repeatable);
}

ERS_TEST_CASE("bad file names", "[flux]") {
    CHECK(rejected("0001_seed.sql"));
    CHECK(rejected("0001_seed.exsiting.sql"));
    CHECK(rejected("1_seed.fresh.sql"));
    CHECK(rejected("Best_Sale.repeatable.sql"));
}

ERS_TEST_CASE("transaction control", "[flux]") {
    CHECK(rejected("0001_seed.fresh.sql", "BEGIN;\nINSERT INTO t VALUES (1);\nCOMMIT;"));
    CHECK(rejected("0001_seed.fresh.sql", "  rollback ;"));

    CHECK_FALSE(rejected("f.repeatable.sql",
        "CREATE FUNCTION f() RETURNS void LANGUAGE plpgsql AS $$\n"
        "BEGIN\n"
        "    PERFORM 1;\n"
        "END;\n"
        "$$;"));
}

ERS_TEST_CASE("directives", "[flux]") {
    const auto migration = parsed("0004_sales_banner.existing.sql",
        "-- flux: table public.sales 3f9a1c0d2b7e4410\n"
        "-- flux: drop public.old_banners\n"
        "ALTER TABLE public.sales ADD COLUMN banner TEXT;");

    REQUIRE(migration.claims.size() == 1);
    CHECK(migration.claims[0].table == "public.sales");
    CHECK(migration.claims[0].hash == 0x3f9a1c0d2b7e4410ull);
    CHECK(migration.drops == std::vector<std::string> { "public.old_banners" });
}

ERS_TEST_CASE("bad directives", "[flux]") {
    CHECK(rejected("0001_seed.fresh.sql", "-- flux: table public.sales 3f9a1c0d2b7e4410\nSELECT 1;"));
    CHECK(rejected("0001_x.existing.sql", "-- flux: tabel public.sales 3f9a1c0d2b7e4410\nSELECT 1;"));
    CHECK(rejected("0001_x.existing.sql", "-- flux: table public.sales 3f9a\nSELECT 1;"));
}


// Outcome and diff ------------------------------------------------------------

ERS_TEST_CASE("outcome", "[flux]") {
    struct row_t {
        EDatabaseState state;
        EApplyOn on;
        u32 version;
        EMigrationEvent expected;
    };

    constexpr u32 baseline = 5;

    constexpr row_t rows[] = {
        { .state = EDatabaseState::Fresh, .on = EApplyOn::Existing, .version = 1, .expected = EMigrationEvent::Stamped },
        { .state = EDatabaseState::Fresh, .on = EApplyOn::Fresh, .version = 1, .expected = EMigrationEvent::Applied },
        { .state = EDatabaseState::Fresh, .on = EApplyOn::Always, .version = 1, .expected = EMigrationEvent::Applied },
        { .state = EDatabaseState::Tracked, .on = EApplyOn::Existing, .version = 1, .expected = EMigrationEvent::Applied },
        { .state = EDatabaseState::Tracked, .on = EApplyOn::Fresh, .version = 1, .expected = EMigrationEvent::Skipped },
        { .state = EDatabaseState::Tracked, .on = EApplyOn::Always, .version = 1, .expected = EMigrationEvent::Applied },
        { .state = EDatabaseState::Legacy, .on = EApplyOn::Existing, .version = 5, .expected = EMigrationEvent::Stamped },
        { .state = EDatabaseState::Legacy, .on = EApplyOn::Fresh, .version = 5, .expected = EMigrationEvent::Stamped },
        { .state = EDatabaseState::Legacy, .on = EApplyOn::Always, .version = 5, .expected = EMigrationEvent::Stamped },
        { .state = EDatabaseState::Legacy, .on = EApplyOn::Existing, .version = 6, .expected = EMigrationEvent::Applied },
        { .state = EDatabaseState::Legacy, .on = EApplyOn::Fresh, .version = 6, .expected = EMigrationEvent::Skipped },
        { .state = EDatabaseState::Legacy, .on = EApplyOn::Always, .version = 6, .expected = EMigrationEvent::Applied },
    };

    for (const auto& [state, on, version, expected] : rows) {
        CAPTURE(version);
        CHECK(decide_outcome(state, { .version = version, .on = on }, baseline) == expected);
    }
}

ERS_TEST_CASE("diff", "[flux]") {
    const auto diff = diff_declarations(
        "CONSTRAINT sales_pk PRIMARY KEY (id)\nid INTEGER NOT NULL\npercent INTEGER NOT NULL",
        "CONSTRAINT sales_pk PRIMARY KEY (id)\nbanner TEXT NOT NULL\nid INTEGER NOT NULL\npercent SMALLINT NOT NULL"
    );

    CHECK(diff == std::vector<std::string> {
        "- percent INTEGER NOT NULL",
        "+ banner TEXT NOT NULL",
        "+ percent SMALLINT NOT NULL",
        });
}


// Migrator --------------------------------------------------------------------

ERS_TEST_CASE("fresh database", "[flux]") {
    Scenario s;
    s.set.versioned = {
        parsed("0001_sales_banner.existing.sql", "ALTER TABLE public.sales ADD COLUMN banner TEXT;"),
        parsed("0002_seed.fresh.sql", "INSERT INTO public.sales VALUES (1);"),
    };
    s.set.repeatable = { parsed("best_sale.repeatable.sql", "CREATE OR REPLACE FUNCTION best_sale();") };

    const auto report = s.run();
    REQUIRE(report);

    CHECK(s.store.executed == std::vector<std::string> {
        "CREATE SCHEMA IF NOT EXISTS public",
        std::string(sales.ddl),
        "INSERT INTO public.sales VALUES (1);",
        "CREATE OR REPLACE FUNCTION best_sale();",
        });

    CHECK(reported(*report, EMigrationEvent::Stamped, "public/0001_sales_banner"));
    CHECK(reported(*report, EMigrationEvent::Applied, "public/0002_seed"));
    CHECK(s.store.recorded.at("public.sales").hash == sales.hash);
}

ERS_TEST_CASE("changed declaration", "[flux]") {
    Scenario s;
    s.track(sales_v1);

    const auto report = s.run();
    REQUIRE_FALSE(report);

    const auto& message = report.error().message();
    CHECK(message.contains("+ banner TEXT NOT NULL"));
    CHECK(message.contains(claim("public.sales", sales_v2).substr(0, 41)));
}

ERS_TEST_CASE("claiming migration", "[flux]") {
    Scenario s;
    s.track(sales_v1);
    s.set.versioned = {
        parsed("0001_sales_banner.existing.sql",
            claim("public.sales", sales_v2) + "ALTER TABLE public.sales ADD COLUMN banner TEXT NOT NULL DEFAULT '';")
    };

    REQUIRE(s.run());

    CHECK(s.store.executed.front() == s.set.versioned.front().sql);
    CHECK(s.store.recorded.at("public.sales").text == sales_v2);
}

ERS_TEST_CASE("foreign migration", "[flux]") {
    Scenario s;
    s.track(sales_v1);
    s.set.versioned = {
        parsed("0001_sales_other.existing.sql",
            claim("public.sales", "other TEXT NOT NULL") + "SELECT 1;")
    };

    const auto report = s.run();
    REQUIRE_FALSE(report);
    CHECK(report.error().message().contains("Recorded declaration is unknown"));
}

ERS_TEST_CASE("untracked database", "[flux]") {
    Scenario s;
    s.store.existing_tables.emplace_back(sales.name);
    s.set.versioned = { parsed("0001_seed.fresh.sql") };

    CHECK_FALSE(s.run());

    s.options.adopt = true;
    const auto report = s.run();
    REQUIRE(report);

    CHECK(reported(*report, EMigrationEvent::Adopted, "public"));
    CHECK(reported(*report, EMigrationEvent::Skipped, "public/0001_seed"));
    CHECK(s.store.recorded.contains("public.sales"));
}

ERS_TEST_CASE("removed table", "[flux]") {
    Scenario s;
    s.track(sales_v2);
    s.store.recorded.insert_or_assign("public.banners", recorded_table_t {
        .format = declaration_format,
        .hash   = ers::Fnv1AHash<std::string_view> {}("id INTEGER NOT NULL"),
        .text   = "id INTEGER NOT NULL",
    });

    const auto failed = s.run();
    REQUIRE_FALSE(failed);
    CHECK(failed.error().message().contains("-- flux: drop public.banners"));

    s.set.versioned = {
        parsed("0001_drop_banners.existing.sql",
            "-- flux: drop public.banners\nDROP TABLE public.banners;")
    };

    REQUIRE(s.run());
    CHECK_FALSE(s.store.recorded.contains("public.banners"));
}

ERS_TEST_CASE("accept_declarations", "[flux]") {
    Scenario s;
    s.track(sales_v1);
    s.options.accept_declarations = true;

    const auto report = s.run();
    REQUIRE(report);

    CHECK(reported(*report, EMigrationEvent::Accepted, "public.sales"));
    CHECK(s.store.recorded.at("public.sales").hash == sales.hash);
}

ERS_TEST_CASE("repeatable", "[flux]") {
    Scenario s;
    s.track(sales_v2);
    s.set.repeatable = { parsed("f.repeatable.sql", "SELECT 1;") };

    REQUIRE(s.run());
    s.store.executed.clear();

    REQUIRE(s.run());
    CHECK_FALSE(std::ranges::contains(s.store.executed, "SELECT 1;"sv));

    s.set.repeatable = { parsed("f.repeatable.sql", "SELECT 2;") };

    REQUIRE(s.run());
    CHECK(std::ranges::contains(s.store.executed, "SELECT 2;"sv));
}

ERS_TEST_CASE("edited migration", "[flux]") {
    Scenario s;
    s.track(sales_v2);
    s.store.migrations.insert_or_assign("0001_seed", ers::Fnv1AHash<std::string_view> {}("SELECT 0;"));
    s.set.versioned = { parsed("0001_seed.always.sql", "SELECT 1;") };

    const auto report = s.run();
    REQUIRE(report);

    CHECK(reported(*report, EMigrationEvent::Edited, "public/0001_seed"));
    CHECK_FALSE(std::ranges::contains(s.store.executed, "SELECT 1;"sv));
}

ERS_TEST_CASE("failing migration", "[flux]") {
    Scenario s;
    s.track(sales_v2);
    s.set.versioned = { parsed("0007_broken.always.sql", "SELECT 1/0;") };
    s.store.failing_sql = "SELECT 1/0;";

    const auto report = s.run();
    REQUIRE_FALSE(report);

    CHECK(report.error().message().contains("public/0007_broken"));
    CHECK_FALSE(s.store.migrations.contains("0007_broken"));
}


// Reflection ------------------------------------------------------------------

#ifdef ERSLIB_HAS_REFLECTION

#include <erslib/flux/reflect.hpp>

namespace {
    struct SaleOrdered {
        u32 id;
        std::string code;
        bool enabled = true;
    };

    struct SaleShuffled {
        bool enabled = true;
        std::string code;
        u32 id;
    };

    struct SaleDisabledByDefault {
        u32 id;
        std::string code;
        bool enabled = false;
    };
}

template<>
struct flux::reflect::Declaration<SaleOrdered> : Table<"sales"> {
    Pk<"id"> sales_pk;
    Default<"enabled", true> enabled_default;
};

template<>
struct flux::reflect::Declaration<SaleShuffled> : Table<"sales"> {
    Pk<"id"> sales_pk;
    Default<"enabled", true> enabled_default;
};

template<>
struct flux::reflect::Declaration<SaleDisabledByDefault> : Table<"sales"> {
    Pk<"id"> sales_pk;
    Default<"enabled", false> enabled_default;
};


ERS_TEST_CASE("canonical declaration", "[flux]") {
    using flux::ddl::canonical_declaration;

    static_assert(canonical_declaration<SaleOrdered>() == canonical_declaration<SaleShuffled>());
    static_assert(canonical_declaration<SaleOrdered>() != canonical_declaration<SaleDisabledByDefault>());

    CHECK(canonical_declaration<SaleOrdered>().contains("enabled BOOLEAN NOT NULL DEFAULT TRUE"));
    CHECK(canonical_declaration<SaleOrdered>().contains("CONSTRAINT sales_pk PRIMARY KEY (id)"));
}

#endif
