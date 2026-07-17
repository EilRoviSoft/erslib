#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// std
#include <cstdint>
#include <filesystem>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/dbio.hpp>

// generated
#include "stats.g.hpp"
#include "user.g.hpp"


namespace fs = std::filesystem;


TEST_CASE("dbio codegen: generated SQL loads from the query store") {
    dbio::QueryStore queries;

    const auto loaded = queries.load_directory(fs::path(TEST_CWD) / "res/query");
    REQUIRE(loaded > 0);

    CHECK(queries.get("user.create").has_value());
    CHECK(queries.get("user.save").has_value());
    CHECK(queries.get("user.load_by_id").has_value());
    CHECK(queries.get("user.load_by_name").has_value());

    SUBCASE("save is an upsert") {
        const auto save = queries.get("user.save");
        REQUIRE(save.has_value());
        CHECK(save->find("INSERT") != std::string::npos);
        CHECK(save->find("ON CONFLICT") != std::string::npos);
    }

    SUBCASE("placeholders are 1-based") {
        const auto load = queries.get("user.load_by_id");
        REQUIRE(load.has_value());
        CHECK(load->find("$1") != std::string::npos);
        CHECK(load->find("$0") == std::string::npos);
    }
}


TEST_CASE("dbio codegen: generated entity layout") {
    CHECK(static_cast<std::size_t>(app::UserField::Max) == 3);

    const app::User user {};
    CHECK(user.id == dbio::undefined_id);
}


TEST_CASE("dbio codegen: generated make_config") {
    constexpr auto in_all = app::user::make_config<dbio::In>(app::UserLayout::All);
    CHECK(in_all[app::UserField::Id]);
    CHECK(in_all[app::UserField::Name]);
    CHECK(in_all[app::UserField::Age]);

    constexpr auto out_save = app::user::make_config<dbio::Out>(app::UserLayout::Save);
    CHECK_FALSE(out_save[app::UserField::Id]);
    CHECK(out_save[app::UserField::Name]);
    CHECK(out_save[app::UserField::Age]);
}


TEST_CASE("dbio codegen: custom SQL queries load from the query store") {
    const auto loaded = dbio::queries.load_directory(fs::path(TEST_CWD) / "res/query");
    REQUIRE(loaded > 0);

    CHECK(dbio::queries.get("user.select_adults").has_value());

    CHECK(dbio::queries.get("stats.age_histogram").has_value());
    CHECK(dbio::queries.get("stats.oldest_user").has_value());
    CHECK(dbio::queries.get("stats.rename_user").has_value());

    SUBCASE("select layout honors order_by / limit and is 1-based") {
        const auto sql = dbio::queries.get("user.select_adults");
        REQUIRE(sql.has_value());
        CHECK(sql->find("ORDER BY") != std::string::npos);
        CHECK(sql->find("$1") != std::string::npos);
    }

    SUBCASE("standalone aggregate query") {
        const auto sql = dbio::queries.get("stats.age_histogram");
        REQUIRE(sql.has_value());
        CHECK(sql->find("GROUP BY") != std::string::npos);
    }

    SUBCASE("statement sourced from an external .sql file") {
        const auto sql = dbio::queries.get("stats.rename_user");
        REQUIRE(sql.has_value());
        CHECK(sql->find("UPDATE") != std::string::npos);
    }
}


TEST_CASE("dbio codegen: custom query result structs") {
    static_assert(std::is_same_v<decltype(app::AgeHistogram::age), int32_t>);
    static_assert(std::is_same_v<decltype(app::AgeHistogram::user_count), int64_t>);
    static_assert(std::is_same_v<decltype(app::OldestUser::id), uint32_t>);
    static_assert(std::is_same_v<decltype(app::OldestUser::name), std::string>);

    constexpr app::AgeHistogram histogram;
    CHECK(histogram.age == 0);
    CHECK(histogram.user_count == 0);

    const dbio::RowGenerator<app::AgeHistogram> generator;
    CHECK(generator.size() == 0);
}


TEST_CASE("dbio codegen: custom query function signatures") {
    using histogram_ret = decltype(app::stats::age_histogram(
        std::declval<pqxx::dbtransaction&>(),
        std::declval<int32_t>()
    ));

    static_assert(std::is_same_v<histogram_ret, ers::Result<app::stats::AgeHistogramGenerator>>);


    using oldest_ret = decltype(app::stats::oldest_user(std::declval<pqxx::dbtransaction&>()));

    static_assert(std::is_same_v<oldest_ret, ers::Result<app::OldestUser>>);


    using rename_ret = decltype(app::stats::rename_user(
        std::declval<pqxx::dbtransaction&>(),
        std::declval<uint32_t>(),
        std::declval<std::string_view>()
    ));
    static_assert(std::is_same_v<rename_ret, ers::Status>);
}
