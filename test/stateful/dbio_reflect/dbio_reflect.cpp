#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

// std
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <string>

// doctest
#include <doctest/doctest.h>

// ers
#include <erslib/dbio.hpp>
#include <erslib/dbio/query.hpp>

#ifdef ERSLIB_HAS_REFLECTION
#include <erslib/dbio/reflect.hpp>
#endif


namespace fs = std::filesystem;


TEST_CASE("dbio store: hand-written queries load from a directory") {
    dbio::QueryStore queries;

    const auto loaded = queries.load_directory(fs::path(TEST_QUERY_DIR));
    REQUIRE(loaded > 0);

    const auto rename = queries.get("rename_user");
    REQUIRE(rename.has_value());

    CHECK(rename->find("UPDATE users") != std::string::npos);
    CHECK(rename->find("$1") != std::string::npos);
    CHECK(rename->find("$0") == std::string::npos);
}


#ifdef ERSLIB_HAS_REFLECTION

namespace {
    struct Image {
        uint32_t id = dbio::undefined_id;
        std::string url;
        std::string path;
    };

    struct ImageTag {
        uint32_t image_id = dbio::undefined_id;
        uint32_t tag_id = dbio::undefined_id;
        uint16_t position;
    };
}

template<>
struct dbio::reflect::Declaration<Image> : Table<"reflect_images"> {
    Pk<"id"> reflect_images_pk;
    Unique<"url"> reflect_images_url_key;
};

template<>
struct dbio::reflect::Declaration<ImageTag> : Table<"reflect_image_tags"> {
    Pk<"image_id", "tag_id"> reflect_image_tags_pk;
};


namespace {
    // Set ERSLIB_TEST_PG_DSN to a libpq connection string to exercise the round trip;
    // without it there is no server to talk to and this case is skipped.
    const char* dsn() {
        return std::getenv("ERSLIB_TEST_PG_DSN");
    }
}


TEST_CASE("dbio reflect: round trip against postgres" * doctest::skip(dsn() == nullptr)) {
    dbio::Database db(dsn());

    auto done = db.with_transaction([](pqxx::work& tx) -> ers::Status {
        ERS_QUICK_DBIO_USING;
        namespace refl = dbio::reflect;

        tx.exec("DROP TABLE IF EXISTS reflect_image_tags");
        tx.exec("DROP TABLE IF EXISTS reflect_images");
        tx.exec(std::string(dbio::ddl::create_table<Image>()));
        tx.exec(std::string(dbio::ddl::create_table<ImageTag>()));

        // The identity column is server-generated, so it stays out of the insert and
        // comes back through RETURNING.
        auto inserted = (insert_into("reflect_images")
            | columns("url", "path")
            | values(std::string("a.png"), std::string("/img/a.png"))
            | returning(std::string(refl::identity_column<Image>))
        ).exec_one<Image>(tx);

        if (!inserted)
            return inserted.error();

        CHECK(inserted->id != dbio::undefined_id);

        // RETURNING carried only id, so every other column was left at its default.
        CHECK(inserted->url.empty());

        const uint32_t id = inserted->id;

        // Upsert on the natural key: same row, new path.
        auto again = (insert_into("reflect_images")
            | columns("url", "path")
            | values(std::string("a.png"), std::string("/img/a-v2.png"))
            | on_conflict_constraint("reflect_images_url_key")
            | do_update("path")
            | returning("id")
        ).exec_one<Image>(tx);

        if (!again)
            return again.error();

        CHECK(again->id == id);

        // SELECT * fills the entity by column name.
        auto loaded = (select_from("reflect_images") | where("id", id)).exec_one<Image>(tx);

        if (!loaded)
            return loaded.error();

        CHECK(loaded->url == "a.png");
        CHECK(loaded->path == "/img/a-v2.png");

        auto all = (select_from("reflect_images")).exec_as<Image>(tx);

        if (!all)
            return all.error();

        CHECK(all->size() == 1);
        CHECK((*all->begin()).url == "a.png");

        // Composite key, no identity column. The constraint name comes from the declaration.
        auto upsert_tag = [&](uint16_t position) {
            return (insert_into("reflect_image_tags")
                | columns("image_id", "tag_id", "position")
                | values(id, uint32_t(7), position)
                | on_conflict_constraint(std::string(refl::pk_constraint<ImageTag>))
                | do_update("position")
            ).exec_and_discard(tx);
        };

        if (auto s = upsert_tag(1); !s)
            return s;

        if (auto s = upsert_tag(2); !s)
            return s;

        auto tags = (select_from("reflect_image_tags")).exec_as<ImageTag>(tx);

        if (!tags)
            return tags.error();

        REQUIRE(tags->size() == 1);
        CHECK((*tags->begin()).position == 2);

        if (auto s = (delete_from("reflect_image_tags")
                | where("image_id", id)
                | where("tag_id", 7)
            ).exec_and_discard(tx); !s) {
            return s;
        }

        auto empty = (select_from("reflect_image_tags")).exec_as<ImageTag>(tx);

        if (!empty)
            return empty.error();

        CHECK(empty->size() == 0);

        return ers::ok;
    });

    CHECK(done);
}

#endif
