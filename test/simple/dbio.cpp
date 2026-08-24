// doctest
#include <doctest/doctest.h>

// std
#include <cstdint>
#include <string>
#include <string_view>

// ers
#include <erslib/dbio.hpp>
#include <erslib/dbio/query_builder.hpp>


namespace {
    bool contains(std::string_view haystack, std::string_view needle) {
        return haystack.find(needle) != std::string_view::npos;
    }

    std::string sql_of(const auto& q) {
        auto r = q.to_sql();
        return r ? *r : std::string("<error>");
    }
}


//----------------------------------------------------------------------------------------------------------------------
// query builder
//----------------------------------------------------------------------------------------------------------------------


TEST_CASE("dbio: select renders slots in layout order") {
    ERS_QUICK_DBIO_USING;

    const auto sql = sql_of(select_from("users")
        | columns("id", "name")
        | where("active", true)
        | order_by("created_at", dbio::ext::Order::Desc)
        | with_limit(20)
        | with_offset(5));

    CHECK(sql ==
        "SELECT id, name"
        "\nFROM users"
        "\nWHERE active = $1"
        "\nORDER BY created_at DESC"
        "\nLIMIT $2"
        "\nOFFSET $3");
}

TEST_CASE("dbio: select without columns falls back to *") {
    ERS_QUICK_DBIO_USING;

    CHECK(sql_of(select_from("users")) == "SELECT *\nFROM users");
}

TEST_CASE("dbio: insert with conflict and returning") {
    ERS_QUICK_DBIO_USING;

    const auto sql = sql_of(insert_into("images")
        | columns("url", "path")
        | values(std::string("a.png"), std::string("/a.png"))
        | on_conflict("url")
        | do_update("path")
        | returning("id"));

    CHECK(sql ==
        "INSERT INTO images (url, path)"
        "\nVALUES ($1, $2)"
        "\nON CONFLICT (url)"
        "\nDO UPDATE SET path = excluded.path"
        "\nRETURNING id");
}

TEST_CASE("dbio: update and delete") {
    ERS_QUICK_DBIO_USING;

    CHECK(sql_of(update("users") | assign("name", std::string("bob")) | where("id", 1))
        == "UPDATE users\nSET name = $1\nWHERE id = $2");

    CHECK(sql_of(delete_from("users") | where("id", 1))
        == "DELETE FROM users\nWHERE id = $1");
}

TEST_CASE("dbio: where variants") {
    ERS_QUICK_DBIO_USING;

    CHECK(contains(sql_of(select_from("t") | where_null("a")), "WHERE a IS NULL"));
    CHECK(contains(sql_of(select_from("t") | where_null("a", false)), "WHERE a IS NOT NULL"));
    CHECK(contains(sql_of(select_from("t") | where_in("a", {1, 2, 3})), "WHERE a IN ($1, $2, $3)"));
    CHECK(contains(sql_of(select_from("t") | where_not("a", 1)), "WHERE a <> $1"));

    // "x IN ()" is not valid SQL, so an empty list collapses to a constant.
    CHECK(contains(sql_of(select_from("t") | where_in("a", std::vector<int> {})), "WHERE FALSE"));
    CHECK(contains(sql_of(select_from("t") | where_in("a", std::vector<int> {}, true)), "WHERE TRUE"));
}

TEST_CASE("dbio: identifiers are validated, values are bound") {
    ERS_QUICK_DBIO_USING;

    CHECK_FALSE((select_from("users") | where("id; DROP TABLE users", 1)).to_sql().has_value());
    CHECK_FALSE((select_from("users") | columns("1")).to_sql().has_value());
    CHECK_FALSE(select_from("users; --").to_sql().has_value());

    // qualified names stay legal
    CHECK((select_from("public.users") | where("users.id", 1)).to_sql().has_value());

    // a hostile value is a parameter, never text
    CHECK(contains(sql_of(select_from("t") | where("a", std::string("'; DROP TABLE t; --"))), "WHERE a = $1"));
}

TEST_CASE("dbio: a clause outside the layout is rejected") {
    ERS_QUICK_DBIO_USING;

    // DELETE has no ORDER BY slot
    CHECK_FALSE((delete_from("t") | order_by("a")).to_sql().has_value());
}

TEST_CASE("dbio: subqueries share the parent placeholder counter") {
    ERS_QUICK_DBIO_USING;

    const auto sql = sql_of(select_from("users")
        | columns("id")
        | where("active", true)
        | where_exists(select_from("orders") | where("user_id", 7))
        | with_limit(10));

    CHECK(contains(sql, "WHERE active = $1 AND EXISTS (SELECT *"));
    CHECK(contains(sql, "WHERE user_id = $2"));
    CHECK(contains(sql, "LIMIT $3"));
}

TEST_CASE("dbio: exists_in is a standalone statement") {
    ERS_QUICK_DBIO_USING;

    CHECK(sql_of(exists_in("orders") | where("user_id", 7))
        == "SELECT EXISTS (\nSELECT 1 FROM orders\nWHERE user_id = $1\n) AS \"exists\"");
}


//----------------------------------------------------------------------------------------------------------------------
// reflection
//----------------------------------------------------------------------------------------------------------------------


#ifdef ERSLIB_HAS_REFLECTION

namespace {
    struct Image {
        uint32_t id = dbio::undefined_id;
        std::string url;

        [[=dbio::reflect::Column<"", "VARCHAR(120)">]]
        std::string path;

        [[=dbio::reflect::Skip]]
        std::string cache;
    };

    struct ProductImage {
        uint32_t product_id;
        uint32_t image_id;
        bool replaceable;
    };

    // Aggregate with no Declaration: read positionally by column name.
    struct Plain {
        uint32_t a;
        std::string b;
    };

    // Handwritten row reader on a reflected entity.
    struct HandRolled {
        int id;
        explicit HandRolled(pqxx::row_ref) : id(0) {}
    };

    // A constraint kind defined entirely outside the library: it only declares
    // the category tag and how to render itself.
    template<ers::fixed_string Expr>
    struct Check {
        using table_constraint_tag = void;

        static consteval std::string ddl(std::string_view name) {
            return "CONSTRAINT " + std::string(name) + " CHECK (" + std::string(Expr.to_sv()) + ')';
        }
    };
}

template<>
struct dbio::reflect::Declaration<Image> : Table<"images"> {
    Pk<"id"> images_pk;
    Unique<"url"> images_url_key;
};

template<>
struct dbio::reflect::Declaration<ProductImage> : Table<"product_images"> {
    Pk<"product_id", "image_id"> product_images_pk;
    Fk<"image_id", "images", "id", action_on_delete::cascade> product_images_image_fk;
    Default<"replaceable", true> product_images_replaceable;
    Check<"image_id > 0"> product_images_image_check;
};

template<>
struct dbio::reflect::Declaration<HandRolled> : Table<"hand_rolled"> {};


TEST_CASE("dbio reflect: schema facts") {
    using namespace dbio::reflect;

    static_assert(table_name<Image> == "images");
    static_assert(Entity<Image>);
    static_assert(!Statement<Image>);

    static_assert(column_count<Image> == 3); // `cache` is skipped
    static_assert(has_field<Image>("cache"));
    static_assert(!has_column<Image>("cache"));
    static_assert(has_column<Image>("path"));

    static_assert(has_identity<Image>);
    static_assert(identity_column<Image> == "id");
    static_assert(!has_identity<ProductImage>); // composite key

    static_assert(declaration_is_valid<Image>);
    static_assert(declaration_is_valid<ProductImage>);
}

TEST_CASE("dbio reflect: row concepts") {
    using namespace dbio::reflect;

    static_assert(RowType<Image> && dbio::ReadableRow<Image>);
    static_assert(dbio::impl::reflect::PlainRow<Plain> && dbio::ReadableRow<Plain>);
    static_assert(std::is_same_v<dbio::row_reader<Image>::state, ColumnIndex<Image>>);

    // std::string is a single-column scalar read, not an entity row
    static_assert(!RowType<std::string>);
    static_assert(!dbio::impl::reflect::PlainRow<std::string>);
    static_assert(dbio::ReadableRow<std::string>);

    // an explicit T(pqxx::row_ref) beats reflection, so the two readers stay unambiguous
    static_assert(RowType<HandRolled>);
    static_assert(!dbio::impl::reflect::MappedRow<HandRolled>);
    static_assert(std::is_empty_v<dbio::row_reader<HandRolled>::state>);
}

TEST_CASE("dbio reflect: create_table") {
    constexpr std::string_view images = dbio::ddl::create_table<Image>();

    CHECK(contains(images, "CREATE TABLE IF NOT EXISTS images"));
    CHECK(contains(images, "id INTEGER GENERATED ALWAYS AS IDENTITY"));
    CHECK(contains(images, "path VARCHAR(120) NOT NULL")); // Column<> type override
    CHECK_FALSE(contains(images, "cache"));                // Skip
    CHECK(contains(images, "CONSTRAINT images_url_key UNIQUE (url)"));

    constexpr std::string_view product_images = dbio::ddl::create_table<ProductImage>();

    CHECK(contains(product_images, "replaceable BOOLEAN NOT NULL DEFAULT TRUE"));
    CHECK(contains(product_images, "CONSTRAINT product_images_pk PRIMARY KEY (product_id, image_id)"));
    CHECK(contains(product_images,
        "CONSTRAINT product_images_image_fk FOREIGN KEY (image_id) "
        "REFERENCES images (id) ON DELETE CASCADE"));
}

TEST_CASE("dbio reflect: a constraint kind defined outside the library renders") {
    constexpr std::string_view t = dbio::ddl::create_table<ProductImage>();

    CHECK(contains(t, "CONSTRAINT product_images_image_check CHECK (image_id > 0)"));
}

#endif
