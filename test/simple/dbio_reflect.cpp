// doctest
#include <doctest/doctest.h>

#ifdef ERSLIB_HAS_REFLECTION

// std
#include <cstdint>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/dbio/constant.hpp>
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

#endif
