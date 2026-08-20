// doctest
#include <doctest/doctest.h>

#ifdef ERSLIB_HAS_REFLECTION

// std
#include <cstdint>
#include <meta>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/dbio/constant.hpp>
#include <erslib/dbio/query.hpp>
#include <erslib/dbio/reflect.hpp>


namespace {
    std::string squeeze(std::string_view what) {
        std::string out;
        bool pending = false;

        for (const char it : what) {
            if (it == ' ' || it == '\t' || it == '\n' || it == '\r') {
                pending = true;
                continue;
            }

            if (pending && !out.empty())
                out += ' ';

            pending = false;
            out += it;
        }

        return out;
    }

    bool contains(std::string_view text, std::string_view what) {
        return squeeze(text).find(squeeze(what)) != std::string::npos;
    }

    std::string sql_of(const dbio::Query& what) {
        auto out = what.to_sql();
        REQUIRE(out.has_value());
        return squeeze(*out);
    }
}


namespace {
    struct Image {
        uint32_t id = dbio::undefined_id;
        std::string url;
        std::string path;
    };

    struct Product {
        uint32_t id = dbio::undefined_id;
        int32_t price;
        ers::optional<std::string> description;
        std::string title;
        uint32_t sku;
        int cache;
    };

    struct ProductImage {
        uint32_t product_id = dbio::undefined_id;
        uint32_t image_id = dbio::undefined_id;
        uint16_t position;
        bool replaceable = true;
    };

    struct Label {
        uint32_t id = dbio::undefined_id;
        std::string name;
    };

    struct Tag {
        uint32_t id = dbio::undefined_id;
        std::string name;
        std::string secret;
    };

    struct Note {
        uint32_t id = dbio::undefined_id;
        std::string body;
    };
}


template<>
struct dbio::reflect::Declaration<Image> : Table<"images"> {
    Pk<"id"> images_pk;
    Unique<"url"> images_url_key;
    Unique<"path"> images_path_key;

    Conflict<"images_url_key"> images_save_target;
};

template<>
struct dbio::reflect::Declaration<Product> : Table<"products"> {
    Pk<"id"> products_pk;
    Unique<"sku"> products_sku_key;

    Column<"descr"> description;
    Column<"", "VARCHAR(120)"> title;
    Skip cache;
};

template<>
struct dbio::reflect::Declaration<ProductImage> : Table<"product_images"> {
    Pk<"product_id", "image_id"> product_images_pk;
    Fk<"product_id", "products", "id", action_on_delete::cascade> product_images_product_fk;
    Fk<"image_id", "images", "id"> product_images_image_fk;
    Unique<"product_id", "position"> product_images_position_key;
    Default<"replaceable", true> product_images_replaceable;
};

template<>
struct dbio::reflect::Declaration<Label> : Table<"labels"> {
    Pk<"id"> labels_pk;
    Unique<"name"> labels_name_key;
};

template<>
struct dbio::reflect::Declaration<Tag> : Table<"tags"> {
    Pk<"id"> tags_pk;
    Unique<"name"> tags_name_key;
};

template<>
struct dbio::reflect::Declaration<Note> : Table<"notes"> {
    Pk<"id"> notes_pk;
    Unique<"body"> notes_body_key;
};


// Extension points

template<>
struct dbio::sql::column_policy<Tag, dbio::sql::kind::Select, dbio::sql::role::Read> {
    template<std::meta::info M>
    static consteval bool includes() {
        return dbio::reflect::column_name<Tag, M>() != "secret";
    }
};

template<>
struct dbio::sql::statement<Note, dbio::sql::kind::Save> {
    static dbio::Query build(const Note& what) {
        ERS_QUICK_DBIO_USING;

        dbio::Query out = insert_into("notes");
        out |= columns("body");
        out |= values(what.body);
        out |= on_conflict("body");
        out |= do_nothing();

        return out;
    }
};


TEST_CASE("dbio reflect: squeeze") {
    CHECK(squeeze("  a\t\tb\n\n  c  ") == "a b c");
    CHECK(squeeze("\n\n") == "");
    CHECK(squeeze("a") == "a");
}

TEST_CASE("dbio reflect: shape") {
    using namespace dbio::reflect;

    CHECK(table_name<Image>() == "images");
    CHECK(table_name<ProductImage>() == "product_images");

    static_assert(declaration_is_valid<Image>());
    static_assert(declaration_is_valid<Product>());
    static_assert(declaration_is_valid<ProductImage>());

    CHECK(primary_key<ProductImage>().size() == 2);

    // Column renames the field; Skip drops it from the table.
    CHECK(column_name<Product, ^^Product::description>() == "descr");
    CHECK(column_name<Product, ^^Product::price>() == "price");
    CHECK(sql_type_name<Product, ^^Product::title>() == "VARCHAR(120)");

    static_assert(has_field<Product>("cache"));
    static_assert(!has_column<Product>("cache"));
    static_assert(column_count<Product>() == 5);
}

TEST_CASE("dbio reflect: identity and conflict target") {
    using namespace dbio::reflect;

    static_assert(has_identity<Image>());
    static_assert(identity_column<Image>() == "id");
    static_assert(!has_identity<ProductImage>());

    static_assert(conflict_constraint<Image>() == "images_url_key");
    static_assert(conflict_constraint<Product>() == "products_sku_key");
    static_assert(conflict_constraint<ProductImage>() == "product_images_pk");

    static_assert(conflict_columns<Image>().size() == 1);
    static_assert(conflict_columns<ProductImage>().size() == 2);
}

TEST_CASE("dbio reflect: ddl for a surrogate key") {
    constexpr std::string_view text = dbio::ddl::create_table<Image>();

    CHECK(squeeze(text) == squeeze(
        "CREATE TABLE IF NOT EXISTS images ("
        "    id INTEGER GENERATED ALWAYS AS IDENTITY,"
        "    url TEXT NOT NULL,"
        "    path TEXT NOT NULL,"
        "    CONSTRAINT images_pk PRIMARY KEY (id),"
        "    CONSTRAINT images_url_key UNIQUE (url),"
        "    CONSTRAINT images_path_key UNIQUE (path)"
        " )"));
}

TEST_CASE("dbio reflect: ddl with nullable, rename and skip") {
    constexpr std::string_view text = dbio::ddl::create_table<Product>();

    CHECK(squeeze(text) == squeeze(
        "CREATE TABLE IF NOT EXISTS products ("
        "    id INTEGER GENERATED ALWAYS AS IDENTITY,"
        "    price INTEGER NOT NULL,"
        "    descr TEXT,"
        "    title VARCHAR(120) NOT NULL,"
        "    sku INTEGER NOT NULL,"
        "    CONSTRAINT products_pk PRIMARY KEY (id),"
        "    CONSTRAINT products_sku_key UNIQUE (sku)"
        " )"));

    CHECK_FALSE(contains(text, "cache"));
}

TEST_CASE("dbio reflect: ddl with composite key and defaults") {
    constexpr std::string_view text = dbio::ddl::create_table<ProductImage>();

    CHECK_FALSE(contains(text, "GENERATED ALWAYS AS IDENTITY"));

    CHECK(contains(text, "replaceable BOOLEAN NOT NULL DEFAULT TRUE"));
    CHECK(contains(text, "CONSTRAINT product_images_pk PRIMARY KEY (product_id, image_id)"));
    CHECK(contains(text,
        "CONSTRAINT product_images_product_fk FOREIGN KEY (product_id) "
        "REFERENCES products (id) ON DELETE CASCADE"));
    CHECK(contains(text,
        "CONSTRAINT product_images_image_fk FOREIGN KEY (image_id) REFERENCES images (id)"));
}


TEST_CASE("dbio sql: select") {
    CHECK(sql_of(dbio::sql::select_all<Image>()) == "SELECT id, url, path FROM images");

    CHECK(sql_of(dbio::sql::load_by_pk<Image>(7u)) ==
        "SELECT id, url, path FROM images WHERE id = $1 LIMIT $2");

    CHECK(sql_of(dbio::sql::load_by_pk<ProductImage>(1u, 2u)) ==
        "SELECT product_id, image_id, position, replaceable FROM product_images "
        "WHERE product_id = $1 AND image_id = $2 LIMIT $3");
}

TEST_CASE("dbio sql: insert skips the identity column and returns it") {
    const Image image {};

    CHECK(sql_of(dbio::sql::insert(image)) ==
        "INSERT INTO images (url, path) VALUES ($1, $2) RETURNING id");
}

TEST_CASE("dbio sql: update matches on the primary key") {
    const Product product {};

    CHECK(sql_of(dbio::sql::update(product)) ==
        "UPDATE products SET price = $1, descr = $2, title = $3, sku = $4 WHERE id = $5");

    const ProductImage product_image {};

    CHECK(sql_of(dbio::sql::update(product_image)) ==
        "UPDATE product_images SET position = $1, replaceable = $2 "
        "WHERE product_id = $3 AND image_id = $4");
}

TEST_CASE("dbio sql: delete matches on the primary key") {
    const ProductImage product_image {};

    CHECK(sql_of(dbio::sql::remove(product_image)) ==
        "DELETE FROM product_images WHERE product_id = $1 AND image_id = $2");
}

TEST_CASE("dbio sql: save upserts on the marked constraint") {
    const Image image {};

    CHECK(sql_of(dbio::sql::save(image)) ==
        "INSERT INTO images (url, path) VALUES ($1, $2) "
        "ON CONFLICT ON CONSTRAINT images_url_key "
        "DO UPDATE SET path = excluded.path "
        "RETURNING id");
}

TEST_CASE("dbio sql: save upserts on a sole unique constraint") {
    const Product product {};

    CHECK(sql_of(dbio::sql::save(product)) ==
        "INSERT INTO products (price, descr, title, sku) VALUES ($1, $2, $3, $4) "
        "ON CONFLICT ON CONSTRAINT products_sku_key "
        "DO UPDATE SET price = excluded.price, descr = excluded.descr, title = excluded.title "
        "RETURNING id");
}

TEST_CASE("dbio sql: save on a composite key upserts on the pk and returns nothing") {
    const ProductImage pi {};
    const std::string text = sql_of(dbio::sql::save(pi));

    CHECK(text ==
        "INSERT INTO product_images (product_id, image_id, position, replaceable) "
        "VALUES ($1, $2, $3, $4) "
        "ON CONFLICT ON CONSTRAINT product_images_pk "
        "DO UPDATE SET position = excluded.position, replaceable = excluded.replaceable");

    CHECK_FALSE(contains(text, "RETURNING"));
}

TEST_CASE("dbio sql: save degrades to DO NOTHING when there is nothing to set") {
    const Label label {};

    CHECK(sql_of(dbio::sql::save(label)) ==
        "INSERT INTO labels (name) VALUES ($1) "
        "ON CONFLICT ON CONSTRAINT labels_name_key "
        "DO NOTHING "
        "RETURNING id");
}


TEST_CASE("dbio sql: column_policy can be specialized per entity") {
    CHECK(sql_of(dbio::sql::select_all<Tag>()) == "SELECT id, name FROM tags");

    const Tag tag {};
    CHECK(contains(sql_of(dbio::sql::save(tag)), "INSERT INTO tags (name, secret)"));
}

TEST_CASE("dbio sql: statement can be specialized per entity") {
    const Note note {};

    CHECK(sql_of(dbio::sql::save(note)) ==
        "INSERT INTO notes (body) VALUES ($1) ON CONFLICT (body) DO NOTHING");

    CHECK(sql_of(dbio::sql::select_all<Note>()) == "SELECT id, body FROM notes");
}

#endif
