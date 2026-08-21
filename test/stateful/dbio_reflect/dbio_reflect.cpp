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
#include <erslib/dbio/query_builder.hpp>

#ifdef ERSLIB_HAS_REFLECTION
#include <erslib/dbio/reflect.hpp>
#endif


namespace fs = std::filesystem;
namespace dbrefl = dbio::reflect;


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

    struct CascadeHitResult {
        uint32_t image_id;
        uint32_t tag_id;
    };
}

namespace query {
    struct CascadeSearch;
    inline constexpr auto cascade_search = dbio::reflect::query_fn<CascadeSearch>;
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

template<>
struct dbio::reflect::Declaration<CascadeHitResult> {};

template<>
struct dbio::reflect::Declaration<query::CascadeSearch> : dbio::reflect::Query<"cascade_search"> {
    struct Input { uint32_t image_id; };
    using Output = CascadeHitResult;
};


namespace {
    const char* dsn() {
        return std::getenv("ERSLIB_TEST_PG_DSN");
    }
}


TEST_CASE("dbio reflect: round trip against postgres" * doctest::skip(dsn() == nullptr)) {
    dbio::Database db(dsn());

    auto done = db.with_transaction([](pqxx::work& tx) -> ers::Status {
        ERS_QUICK_DBIO_USING;

        tx.exec("DROP TABLE IF EXISTS reflect_image_tags");
        tx.exec("DROP TABLE IF EXISTS reflect_images");
        tx.exec(std::string(dbio::ddl::create_table<Image>()));
        tx.exec(std::string(dbio::ddl::create_table<ImageTag>()));

        auto inserted = (insert_into("reflect_images")
            | columns("url", "path")
            | values(std::string("a.png"), std::string("/img/a.png"))
            | returning(std::string(dbrefl::identity_column<Image>))
        ).exec_one<Image>(tx);

        if (!inserted)
            return inserted.error();

        CHECK(inserted->id != dbio::undefined_id);
        CHECK(inserted->url.empty());

        const uint32_t id = inserted->id;

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

        auto upsert_tag = [&](uint16_t position) {
            return (insert_into("reflect_image_tags")
                | columns("image_id", "tag_id", "position")
                | values(id, static_cast<uint32_t>(7), position)
                | on_conflict_constraint(std::string(dbrefl::pk_constraint<ImageTag>))
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

        dbio::QueryStore local_queries;
        local_queries.load_directory(fs::path(TEST_QUERY_DIR));

        auto hits = query::cascade_search(id).exec(tx, local_queries);

        if (!hits)
            return hits.error();

        REQUIRE(hits->size() == 1);
        CHECK((*hits->begin()).image_id == id);
        CHECK((*hits->begin()).tag_id == 7);

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
