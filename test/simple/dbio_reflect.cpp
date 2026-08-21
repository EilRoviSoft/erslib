// doctest
#include <doctest/doctest.h>

#ifdef ERSLIB_HAS_REFLECTION

// std
#include <cstdint>
#include <meta>
#include <string>
#include <string_view>
#include <type_traits>

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/dbio/constant.hpp>
#include <erslib/dbio/database.hpp>
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

        [[=dbio::reflect::Column<"descr">]]
        ers::optional<std::string> description;

        [[=dbio::reflect::Column<"", "VARCHAR(120)">]]
        std::string title;

        uint32_t sku;

        [[=dbio::reflect::Skip]]
        int cache;
    };

    struct ProductImage {
        uint32_t product_id = dbio::undefined_id;
        uint32_t image_id = dbio::undefined_id;
        uint16_t position;
        bool replaceable = true;
    };

    struct CascadeHit {
        std::string source_table;
        uint32_t id;
        uint32_t image_id;
    };

    struct Counted {
        explicit Counted(pqxx::row_ref row) :
            total(row[0].as<int64_t>()) {
        }

        int64_t total;
    };
}


template<>
struct dbio::reflect::Declaration<Image> : Table<"images"> {
    Pk<"id"> images_pk;
    Unique<"url"> images_url_key;
    Unique<"path"> images_path_key;
};

template<>
struct dbio::reflect::Declaration<Product> : Table<"products"> {
    Pk<"id"> products_pk;
    Unique<"sku"> products_sku_key;
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
struct dbio::reflect::Declaration<CascadeHit> {};


TEST_CASE("dbio reflect: squeeze") {
    CHECK(squeeze("  a\t\tb\n\n  c  ") == "a b c");
    CHECK(squeeze("\n\n") == "");
    CHECK(squeeze("a") == "a");
}

TEST_CASE("dbio reflect: shape") {
    using namespace dbio::reflect;

    CHECK(table_name<Image> == "images");
    CHECK(table_name<ProductImage> == "product_images");

    static_assert(declaration_is_valid<Image>);
    static_assert(declaration_is_valid<Product>);
    static_assert(declaration_is_valid<ProductImage>);

    CHECK(primary_key<ProductImage>.size() == 2);
    CHECK(pk_constraint<ProductImage> == "product_images_pk");

    CHECK(column_name<^^Product::description> == "descr");
    CHECK(column_name<^^Product::price> == "price");
    CHECK(sql_type_name<^^Product::title> == "VARCHAR(120)");

    static_assert(has_field<Product>("cache"));
    static_assert(!has_column<Product>("cache"));
    static_assert(column_count<Product> == 5);
}

TEST_CASE("dbio reflect: identity") {
    using namespace dbio::reflect;

    static_assert(has_identity<Image>);
    static_assert(identity_column<Image> == "id");
    static_assert(!has_identity<ProductImage>);
}

TEST_CASE("dbio reflect: concepts") {
    using namespace dbio::reflect;

    static_assert(RowType<Image>);
    static_assert(Entity<Image>);

    static_assert(RowType<CascadeHit>);
    static_assert(!Entity<CascadeHit>);

    static_assert(!RowType<std::string>);
    static_assert(!Entity<std::string>);
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


TEST_CASE("dbio reflect: entities are readable rows") {
    static_assert(dbio::ReadableRow<Image>);
    static_assert(dbio::ReadableRow<ProductImage>);
    static_assert(dbio::ReadableRow<CascadeHit>);

    static_assert(dbio::ValidRow<Counted>);
    static_assert(dbio::ReadableRow<Counted>);

    static_assert(!dbio::ReadableRow<std::string>);

    static_assert(std::is_same_v<dbio::row_reader<Image>::state, dbio::reflect::ColumnIndex<Image>>);
    static_assert(std::is_empty_v<dbio::row_reader<Counted>::state>);
}

namespace {
    [[maybe_unused]]
    auto select_randomly(dbio::Database& db, int64_t limit) {
        return db.with_transaction([limit](pqxx::read_transaction& tx) {
            ERS_QUICK_DBIO_USING;
            return (select_from("images")
                | order_by_random()
                | with_limit(limit)
            ).exec_as<Image>(tx);
        }, "Image/SelectRandomly");
    }
}


TEST_CASE("dbio reflect: hand-built queries") {
    ERS_QUICK_DBIO_USING;

    const Image img {};

    CHECK(sql_of(select_from("images") | columns("id", "url", "path")) ==
        "SELECT id, url, path FROM images");

    CHECK(sql_of(select_from("collections") | order_by_random() | with_limit(10)) ==
        "SELECT * FROM collections ORDER BY RANDOM() LIMIT $1");

    // Column names come from the declaration, so a rename cannot drift out of sync.
    CHECK(sql_of(select_from("products")
        | column(std::string(dbio::reflect::column_name<^^Product::description>))) ==
        "SELECT descr FROM products");

    CHECK(sql_of(insert_into("images")
        | columns("url", "path")
        | values(img.url, img.path)
        | on_conflict_constraint("images_url_key")
        | do_update("path")
        | returning("id")) ==
        "INSERT INTO images (url, path) VALUES ($1, $2) "
        "ON CONFLICT ON CONSTRAINT images_url_key "
        "DO UPDATE SET path = excluded.path "
        "RETURNING id");

    CHECK(sql_of(update("product_images")
        | assign("position", 3)
        | where("product_id", 1)
        | where("image_id", 2)) ==
        "UPDATE product_images SET position = $1 WHERE product_id = $2 AND image_id = $3");

    CHECK(sql_of(delete_from("images") | where("id", 5)) ==
        "DELETE FROM images WHERE id = $1");
}


namespace {
    struct Money {
        int64_t cents = 0;
    };

    struct Invoice {
        uint32_t id = dbio::undefined_id;
        Money total;
    };
}

template<>
struct dbio::reflect::sql_type<Money> {
    static constexpr std::string_view name = "NUMERIC(12, 2)";
    static constexpr bool nullable = false;
};

template<>
struct dbio::reflect::sql_value<Money> {
    static void bind(pqxx::params& out, const Money& what) {
        out.append(what.cents);
    }

    static Money read(const auto& field) {
        return Money(field.template as<int64_t>());
    }
};

template<>
struct dbio::reflect::Declaration<Invoice> : Table<"invoices"> {
    Pk<"id"> invoices_pk;
};


TEST_CASE("dbio reflect: a user-supplied column type") {
    constexpr std::string_view text = dbio::ddl::create_table<Invoice>();

    CHECK(contains(text, "total NUMERIC(12, 2) NOT NULL"));
    static_assert(dbio::ReadableRow<Invoice>);
}

#endif
