// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <concepts>
#include <cstdint>
#include <string>
#include <vector>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/flux.hpp>
#include <erslib/flux/query_builder.hpp>

// test
#include "support/test_case.hpp"


#ifdef ERSLIB_HAS_REFLECTION

using namespace flux::orm;
using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    struct Image {
        std::uint32_t id = flux::undefined_id;
        std::string url;
        std::string path;
    };

    struct User {
        std::uint32_t id = flux::undefined_id;
        std::string email;
        std::string name;

        [[=flux::reflect::Column<"created_at">]]
        std::string created;

        ers::maybe<std::string> nickname;
    };

    struct ProductImage {
        std::uint32_t product_id;
        std::uint32_t image_id;
        bool replaceable = false;
    };

    struct Note {
        std::uint32_t id = flux::undefined_id;
        std::string tenant;
        std::string text;

        [[=flux::reflect::Column<"deleted_at">]]
        ers::maybe<ers::timestamp_t> deleted;
    };

    struct Preview {
        std::uint32_t id = flux::undefined_id;
        std::string url;
    };


    std::string sql_of(const flux::QueryBuilder& q) {
        auto r = q.to_sql();
        return r ? *r : std::string("<error>");
    }
}


template<>
struct flux::reflect::Declaration<Image> : Table<"images"> {
    Pk<"id"> images_pk;
    Unique<"url"> images_url_key;
};

template<>
struct flux::reflect::Declaration<User> : Table<"users"> {
    Pk<"id"> users_pk;
    Unique<"email"> users_email_key;
};

template<>
struct flux::reflect::DeclarationMetadata<User> {
    ReadOnly<"created_at"> users_created_ro;
};

template<>
struct flux::reflect::Declaration<ProductImage> : Table<"product_images"> {
    Pk<"product_id", "image_id"> product_images_pk;
};


template<>
struct flux::reflect::Declaration<Note> : Table<"notes"> {
    Pk<"id"> notes_pk;
};

template<>
struct flux::reflect::DeclarationMetadata<Note> {
    SoftDelete<"deleted_at"> notes_soft;

    static void scope(QueryBuilder& q) {
        q |= where(flux::columns_of<Note>.tenant == "acme");
    }
};


// Columns ---------------------------------------------------------------------

ERS_TEST_CASE("columns", "[flux]") {
    constexpr auto cols = flux::columns_of<User>;

    static_assert(cols.id.name() == "id");
    static_assert(cols.created.name() == "created_at");
    static_assert(std::same_as<decltype(cols.id)::value_type, std::uint32_t>);

    CHECK(sql_of(select_from("users") | where(cols.email == "a@b.c" && cols.id > 5))
        == "SELECT *\nFROM users\nWHERE email = $1 AND id > $2");

    CHECK(sql_of(select_from("users") | columns(cols.id, cols.email.as("mail")) | where(cols.nickname.is_null()))
        == "SELECT id, email AS mail\nFROM users\nWHERE nickname IS NULL");

    CHECK(sql_of(select_from("users") | order_by(cols.name, EOrder::Desc))
        == "SELECT *\nFROM users\nORDER BY name DESC");
}

ERS_TEST_CASE("alias", "[flux]") {
    const auto u = flux::columns_as<User>("u");
    const auto i = flux::columns_as<Image>("i");

    CHECK(sql_of(select_from("users", "u") | join("images", "i", u.id == i.id) | where(u.name == "bob"))
        == "SELECT *\nFROM users AS u\nJOIN images AS i ON u.id = i.id\nWHERE u.name = $1");
}

ERS_TEST_CASE("compare", "[flux][compile_time]") {
    constexpr auto cols = flux::columns_of<User>;

    constexpr auto compares = []<typename C, typename V>(const C&, const V&) {
        return requires(C c, V v) { c == v; };
    };

    static_assert(compares(cols.id, 5));
    static_assert(compares(cols.email, std::string("x")));
    static_assert(compares(cols.email, "x"));
    static_assert(compares(cols.id, cols.id));
    static_assert(compares(cols.id, flux::columns_of<Image>.id));
    static_assert(!compares(cols.id, "x"));
    static_assert(!compares(cols.email, 5));
}


// Builders --------------------------------------------------------------------

ERS_TEST_CASE("select", "[flux]") {
    CHECK(sql_of(flux::crud::select<Image>()) == "SELECT id, url, path\nFROM images");
    CHECK(sql_of(flux::crud::select<User>()) == "SELECT id, email, name, created_at, nickname\nFROM users");
    CHECK(sql_of(flux::crud::select<Image, Preview>()) == "SELECT id, url\nFROM images");

    CHECK(sql_of(flux::crud::select<Image>() | where(flux::columns_of<Image>.url == "x") | with_limit(1))
        == "SELECT id, url, path\nFROM images\nWHERE url = $1\nLIMIT $2");
}

ERS_TEST_CASE("insert", "[flux]") {
    Image image { .url = "/a", .path = "a" };
    User user { .email = "a@b.c", .name = "bob", .created = "never" };

    CHECK(sql_of(flux::crud::insert(image)) == "INSERT INTO images (url, path)\nVALUES ($1, $2)");
    CHECK(sql_of(flux::crud::insert(user)) == "INSERT INTO users (email, name, nickname)\nVALUES ($1, $2, $3)");

    CHECK(sql_of(flux::crud::insert(ProductImage { .product_id = 1, .image_id = 2 }))
        == "INSERT INTO product_images (product_id, image_id, replaceable)\nVALUES ($1, $2, $3)");

    CHECK(sql_of(flux::crud::insert(image) | returning("id")) == "INSERT INTO images (url, path)\nVALUES ($1, $2)\nRETURNING id");
}

ERS_TEST_CASE("insert set slot", "[flux]") {
    auto q = insert_into("t")
        | flux::impl::make_clause<flux::impl::crud::FailClause>(&flux::impl::slots::set, "boom"s);

    const auto r = q.to_sql();

    REQUIRE_FALSE(r.has_value());
    CHECK(r.error().to_string().find("boom") != std::string::npos);
}

ERS_TEST_CASE("update", "[flux]") {
    Image image { .id = 7, .url = "/a", .path = "a" };
    User user { .id = 3, .email = "a@b.c", .name = "bob" };

    CHECK(sql_of(flux::crud::update(image)) == "UPDATE images\nSET url = $1, path = $2\nWHERE id = $3");
    CHECK(sql_of(flux::crud::update(user)) == "UPDATE users\nSET email = $1, name = $2, nickname = $3\nWHERE id = $4");

    CHECK(sql_of(flux::crud::update(ProductImage { .product_id = 1, .image_id = 2 }))
        == "UPDATE product_images\nSET replaceable = $1\nWHERE product_id = $2 AND image_id = $3");
}

ERS_TEST_CASE("update some", "[flux]") {
    constexpr auto cols = flux::columns_of<Image>;
    Image image { .id = 7, .url = "/a", .path = "a" };

    CHECK(sql_of(flux::crud::update(image, flux::crud::only(cols.path)))
        == "UPDATE images\nSET path = $1\nWHERE id = $2");

    CHECK(sql_of(flux::crud::update(image, flux::crud::only(cols.url, cols.path)))
        == "UPDATE images\nSET url = $1, path = $2\nWHERE id = $3");

    CHECK(sql_of(flux::crud::update(image, flux::crud::only("path")))
        == "UPDATE images\nSET path = $1\nWHERE id = $2");

    CHECK(sql_of(flux::crud::update(image, flux::crud::only("id"))) == "<error>");
    CHECK(sql_of(flux::crud::update(image, flux::crud::only("path", "nope"))) == "<error>");
}

ERS_TEST_CASE("erase", "[flux]") {
    CHECK(sql_of(flux::crud::erase<Image>(42)) == "DELETE FROM images\nWHERE id = $1");
    CHECK(sql_of(flux::crud::erase<ProductImage>(1, 2)) == "DELETE FROM product_images\nWHERE product_id = $1 AND image_id = $2");

    CHECK(sql_of(select_from("images") | where(flux::crud::where_pk<Image>(5))) == "SELECT *\nFROM images\nWHERE id = $1");

    constexpr auto keyed = []<typename... K>(const K&... keys) {
        return requires { flux::crud::where_pk<ProductImage>(keys...); };
    };

    static_assert(keyed(1, 2));
    static_assert(!keyed(1));
    static_assert(!keyed(1, 2, 3));
}

ERS_TEST_CASE("upsert", "[flux]") {
    ProductImage link { .product_id = 1, .image_id = 2, .replaceable = true };
    User user { .email = "a@b.c", .name = "bob" };
    Image image { .url = "/a", .path = "a" };

    CHECK(sql_of(flux::crud::upsert(link))
        == "INSERT INTO product_images (product_id, image_id, replaceable)"
           "\nVALUES ($1, $2, $3)"
           "\nON CONFLICT (product_id, image_id)"
           "\nDO UPDATE SET replaceable = excluded.replaceable");

    CHECK(sql_of(flux::crud::upsert<"users_email_key">(user))
        == "INSERT INTO users (email, name, nickname)"
           "\nVALUES ($1, $2, $3)"
           "\nON CONFLICT ON CONSTRAINT users_email_key"
           "\nDO UPDATE SET email = excluded.email, name = excluded.name, nickname = excluded.nickname");

    CHECK(sql_of(flux::crud::upsert<"images_url_key">(image))
        == "INSERT INTO images (url, path)"
           "\nVALUES ($1, $2)"
           "\nON CONFLICT ON CONSTRAINT images_url_key"
           "\nDO UPDATE SET url = excluded.url, path = excluded.path");
}


// Hidden conditions -----------------------------------------------------------

ERS_TEST_CASE("soft select", "[flux]") {
    CHECK(sql_of(flux::crud::select<Note>())
        == "SELECT id, tenant, text, deleted_at\nFROM notes\nWHERE tenant = $1 AND deleted_at IS NULL");

    CHECK(sql_of(flux::crud::select_all<Note>())
        == "SELECT id, tenant, text, deleted_at\nFROM notes\nWHERE tenant = $1");

    CHECK(sql_of(flux::crud::select<Note>() | where(flux::columns_of<Note>.id == 5))
        == "SELECT id, tenant, text, deleted_at\nFROM notes\nWHERE tenant = $1 AND deleted_at IS NULL AND id = $2");

    CHECK(sql_of(flux::crud::select<Image>()) == "SELECT id, url, path\nFROM images");
}

ERS_TEST_CASE("soft erase", "[flux]") {
    CHECK(sql_of(flux::crud::erase<Note>(5))
        == "UPDATE notes\nSET deleted_at = now()\nWHERE id = $1 AND tenant = $2 AND deleted_at IS NULL");

    CHECK(sql_of(flux::crud::erase_forever<Note>(5))
        == "DELETE FROM notes\nWHERE id = $1 AND tenant = $2");

    CHECK(sql_of(flux::crud::restore<Note>(5))
        == "UPDATE notes\nSET deleted_at = NULL\nWHERE id = $1 AND deleted_at IS NOT NULL AND tenant = $2");

    CHECK(sql_of(flux::crud::erase<Image>(5)) == "DELETE FROM images\nWHERE id = $1");
}

ERS_TEST_CASE("soft update", "[flux]") {
    Note note { .id = 5, .tenant = "acme", .text = "hi" };

    CHECK(sql_of(flux::crud::update(note))
        == "UPDATE notes\nSET tenant = $1, text = $2\nWHERE id = $3 AND tenant = $4 AND deleted_at IS NULL");

    CHECK(sql_of(flux::crud::update(note, flux::crud::only("text")))
        == "UPDATE notes\nSET text = $1\nWHERE id = $2 AND tenant = $3 AND deleted_at IS NULL");

    CHECK(sql_of(flux::crud::insert(note)) == "INSERT INTO notes (tenant, text)\nVALUES ($1, $2)");
}


// Declaration -----------------------------------------------------------------

ERS_TEST_CASE("declaration", "[flux][compile_time]") {
    using namespace flux::reflect;

    static_assert(pk_members<ProductImage>.size() == 2);
    static_assert(pk_members<User>.size() == 1);

    static_assert(insert_members<User>.size() == 3);
    static_assert(update_members<User>.size() == 3);
    static_assert(insert_members<ProductImage>.size() == 3);
    static_assert(update_members<ProductImage>.size() == 1);

    static_assert(is_read_only_column<User, ^^User::created>);
    static_assert(!is_read_only_column<User, ^^User::email>);
    static_assert(declaration_is_valid<User>);
    static_assert(metadata_is_valid<User>);
    static_assert(metadata_is_valid<Image>);

    static_assert(has_soft_delete<Note>);
    static_assert(soft_delete_column<Note> == "deleted_at");
    static_assert(!has_soft_delete<User>);
    static_assert(is_read_only_column<Note, ^^Note::deleted>);
    static_assert(insert_members<Note>.size() == 2);
}


// Executors -------------------------------------------------------------------

namespace {
    // Never called, it has to compile.
    [[maybe_unused]] void executors(pqxx::dbtransaction& tx) {
        Image image { .url = "/a", .path = "a" };
        constexpr auto cols = flux::columns_of<Image>;

        Image saved = flux::crud::insert(tx, image);
        Image merged = flux::crud::upsert<"images_url_key">(tx, image);
        ers::maybe<Image> found = flux::crud::find<Image>(tx, 42);
        ers::maybe<Preview> preview = flux::crud::find<Image, Preview>(tx, 42);
        ers::maybe<ProductImage> link = flux::crud::find<ProductImage>(tx, 1, 2);

        bool updated = flux::crud::update(tx, saved);
        bool partial = flux::crud::update(tx, saved, flux::crud::only(cols.path));
        bool erased = flux::crud::erase<Image>(tx, 42);

        ers::maybe<Note> note = flux::crud::find_all<Note>(tx, 42);
        bool forever = flux::crud::erase_forever<Note>(tx, 42);
        bool restored = flux::crud::restore<Note>(tx, 42);

        (void)merged; (void)found; (void)preview; (void)link; (void)updated; (void)partial; (void)erased;
        (void)note; (void)forever; (void)restored;
    }
}

#endif
