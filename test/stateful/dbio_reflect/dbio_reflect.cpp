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
    // without it there is no server to talk to and these cases are skipped.
    const char* dsn() {
        return std::getenv("ERSLIB_TEST_PG_DSN");
    }
}


TEST_CASE("dbio reflect: round trip against postgres" * doctest::skip(dsn() == nullptr)) {
    dbio::Database db(dsn());

    auto done = db.with_transaction([](pqxx::work& tx) -> ers::Status {
        namespace refl = dbio::reflect;

        tx.exec("DROP TABLE IF EXISTS reflect_image_tags");
        tx.exec("DROP TABLE IF EXISTS reflect_images");
        tx.exec(std::string(dbio::ddl::create_table<Image>()));
        tx.exec(std::string(dbio::ddl::create_table<ImageTag>()));

        Image first {};
        first.url = "a.png";
        first.path = "/img/a.png";

        if (auto s = refl::save(tx, first); !s)
            return s;

        CHECK(first.id != dbio::undefined_id);

        Image again {};
        again.url = "a.png";
        again.path = "/img/a-v2.png";

        if (auto s = refl::save(tx, again); !s)
            return s;

        CHECK(again.id == first.id);

        auto loaded = refl::load_by_pk<Image>(tx, first.id);
        if (!loaded)
            return loaded.error();

        REQUIRE(loaded->has_value());
        CHECK((*loaded)->url == "a.png");
        CHECK((*loaded)->path == "/img/a-v2.png");

        auto all = refl::load_all<Image>(tx);
        if (!all)
            return all.error();

        CHECK(all->size() == 1);

        ImageTag tag {};
        tag.image_id = first.id;
        tag.tag_id = 7;
        tag.position = 1;

        if (auto s = refl::save(tx, tag); !s)
            return s;

        tag.position = 2;

        if (auto s = refl::save(tx, tag); !s)
            return s;

        auto tags = refl::load_all<ImageTag>(tx);
        if (!tags)
            return tags.error();

        REQUIRE(tags->size() == 1);
        CHECK(tags->front().position == 2);

        if (auto s = refl::remove(tx, tag); !s)
            return s;

        auto empty = refl::load_all<ImageTag>(tx);
        if (!empty)
            return empty.error();

        CHECK(empty->empty());

        return ers::ok;
    });

    CHECK(done);
}

#endif
