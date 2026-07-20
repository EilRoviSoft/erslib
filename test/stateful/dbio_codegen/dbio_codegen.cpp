#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// std
#include <cstdint>
#include <filesystem>
#include <string>
#include <utility>

// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/dbio.hpp>

// generated
#include "user.g.hpp"


namespace fs = std::filesystem;


TEST_CASE("dbio codegen: generated SQL loads from the query store") {
    dbio::QueryStore queries;

    const auto loaded = queries.load_directory(fs::path(TEST_QUERY_DIR));
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
