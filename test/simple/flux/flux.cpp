// catch2
#include <catch2/catch_test_macros.hpp>

// std
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

// ers
#include <erslib/flux.hpp>
#include <erslib/flux/query_builder.hpp>

// test
#include "support/test_case.hpp"


using namespace flux::orm;

using std::string_literals::operator ""s;


// Details ---------------------------------------------------------------------

namespace {
    bool contains(std::string_view haystack, std::string_view needle) {
        return haystack.find(needle) != std::string_view::npos;
    }

    std::string sql_of(const auto& q) {
        auto r = q.to_sql();
        return r ? *r : std::string("<error>");
    }
}


// Query builder ---------------------------------------------------------------

ERS_TEST_CASE("select", "[flux]") {
    const auto sql = sql_of(select_from("users")
        | columns("id", "name")
        | where(col("active") == true)
        | order_by("created_at", EOrder::Desc)
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

ERS_TEST_CASE("select all", "[flux]") {
    using namespace flux::orm;

    CHECK(sql_of(select_from("users")) == "SELECT *\nFROM users");
}

ERS_TEST_CASE("insert", "[flux]") {
    const auto sql = sql_of(insert_into("images")
        | columns("url", "path")
        | values("a.png"s, "/a.png"s)
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

ERS_TEST_CASE("update and delete", "[flux]") {
    CHECK(sql_of(update("users") | assign("name", "bob"s) | where(col("id") == 1))
        == "UPDATE users\nSET name = $1\nWHERE id = $2");

    CHECK(sql_of(delete_from("users") | where(col("id") == 1))
        == "DELETE FROM users\nWHERE id = $1");
}

ERS_TEST_CASE("where", "[flux]") {
    CHECK(contains(sql_of(select_from("t") | where(col("a").is_null())), "WHERE a IS NULL"));
    CHECK(contains(sql_of(select_from("t") | where(col("a").is_not_null())), "WHERE a IS NOT NULL"));
    CHECK(contains(sql_of(select_from("t") | where(col("a").in({ 1, 2, 3 }))), "WHERE a IN ($1, $2, $3)"));
    CHECK(contains(sql_of(select_from("t") | where(col("a") != 1)), "WHERE a <> $1"));

    // "x IN ()" is not valid SQL, so an empty list collapses to a constant.
    CHECK(contains(sql_of(select_from("t") | where(col("a").in(std::vector<int> {}))), "WHERE FALSE"));
    CHECK(contains(sql_of(select_from("t") | where(!col("a").in(std::vector<int> {}))), "WHERE NOT FALSE"));
}

ERS_TEST_CASE("join", "[flux]") {
    const auto sql = sql_of(select_from("orders", "o")
        | join("users", "u", col("u.id") == col("o.user_id"))
        | where(col("u.active") == true));

    CHECK(contains(sql, "JOIN users AS u ON u.id = o.user_id"));
    CHECK(contains(sql, "WHERE u.active = $1"));
    CHECK_FALSE((select_from("t") | join("x; --", col("a") == col("b"))).to_sql().has_value());
}

ERS_TEST_CASE("vectors", "[flux]") {
    using flux::impl::sql_value;

    CHECK(sql_value<std::vector<int>>::to_text({ 1, -2, 3 }) == "{1,-2,3}");
    CHECK(sql_value<std::vector<bool>>::to_text({ true, false }) == "{t,f}");
    CHECK(sql_value<std::vector<std::string>>::to_text({}) == "{}");
    CHECK(sql_value<std::vector<std::string>>::to_text({ "a b", "x,y", "q\"t", "b\\s", "", "NULL", "{}" })
        == R"({"a b","x,y","q\"t","b\\s","","NULL","{}"})");
    CHECK(sql_value<std::vector<ers::maybe<std::string>>>::to_text({ "a", ers::none }) == R"({"a",NULL})");
}

ERS_TEST_CASE("identifiers", "[flux]") {
    CHECK_FALSE((select_from("users") | where(col("id; DROP TABLE users") == 1)).to_sql().has_value());
    CHECK_FALSE((select_from("users") | columns("1")).to_sql().has_value());
    CHECK_FALSE(select_from("users; --").to_sql().has_value());

    CHECK((select_from("public.users") | where(col("users.id") == 1)).to_sql().has_value());

    CHECK(contains(
        sql_of(select_from("t")
            | where(col("a") == "'; DROP TABLE t; --"s)),
        "WHERE a = $1"
    ));
}

ERS_TEST_CASE("foreign clause", "[flux]") {
    CHECK_FALSE((delete_from("t") | order_by("a")).to_sql().has_value());
}

ERS_TEST_CASE("subqueries", "[flux]") {
    const auto sql = sql_of(select_from("users")
        | columns("id")
        | where(col("active") == true)
        | where(exists(select_from("orders") | where(col("user_id") == 7)))
        | with_limit(10));

    CHECK(contains(sql, "WHERE active = $1 AND EXISTS (SELECT *"));
    CHECK(contains(sql, "WHERE user_id = $2"));
    CHECK(contains(sql, "LIMIT $3"));
}

ERS_TEST_CASE("exists_in", "[flux]") {
    CHECK(sql_of(exists_in("orders") | where(col("user_id") == 7))
        == "SELECT EXISTS (\nSELECT 1 FROM orders\nWHERE user_id = $1\n) AS \"exists\"");
}

ERS_TEST_CASE("exec", "[flux]") {
    static_assert(flux::ReadableRow<bool>);
    static_assert(flux::impl::RowContainer<std::vector<bool>>);
    static_assert(flux::impl::RowContainer<ers::maybe<bool>>);

    static_assert(std::is_same_v<decltype(std::declval<flux::QueryResult>().get_as<bool>()), bool>);
    static_assert(std::is_same_v<decltype(std::declval<flux::QueryResult>().get_as<std::vector<bool>>()), std::vector<bool>>);
    static_assert(std::is_same_v<decltype(std::declval<flux::QueryResult>().get_as<ers::maybe<bool>>()), ers::maybe<bool>>);
    static_assert(std::is_same_v<decltype(std::declval<flux::QueryResult>().view_as<bool>()), flux::RowGenerator<bool>>);

    static_assert(std::is_same_v<decltype(std::declval<flux::QueryResult>().raw()), const pqxx::result&>);
}

namespace {
    struct FakeExecutable {
        flux::QueryResult exec(pqxx::dbtransaction&) const;
    };

    struct FakeQueryCall {
        using Output = bool;
        flux::QueryResult exec(pqxx::dbtransaction&) const;
    };
}

ERS_TEST_CASE("with_query", "[flux]") {
    static_assert(flux::impl::Executable<flux::QueryBuilder>);
    static_assert(flux::impl::Executable<FakeExecutable>);
    static_assert(!flux::impl::Executable<int>);

    static_assert(!flux::impl::ExecutableWithOutput<flux::QueryBuilder>);
    static_assert(!flux::impl::ExecutableWithOutput<FakeExecutable>);
    static_assert(flux::impl::ExecutableWithOutput<FakeQueryCall>);
}


// Reflection ------------------------------------------------------------------

#ifdef ERSLIB_HAS_REFLECTION

namespace {
    struct Image {
        uint32_t id = flux::undefined_id;
        std::string url;

        [[=flux::reflect::Column<"", "VARCHAR(120)">]]
        std::string path;

        [[=flux::reflect::Skip]]
        std::string cache;
    };

    struct ProductImage {
        uint32_t product_id;
        uint32_t image_id;
        bool replaceable;
    };

    struct Plain {
        uint32_t a;
        std::string b;
    };

    struct HandRolled {
        int id;
        explicit HandRolled(pqxx::row_ref) : id(0) {}
    };

    template<ers::fixed_string Expr>
    struct Check {
        using table_constraint_tag = void;

        static consteval std::string ddl(std::string_view name) {
            return "CONSTRAINT " + std::string(name) + " CHECK (" + std::string(Expr.to_sv()) + ')';
        }
    };
}

template<>
struct flux::reflect::Declaration<Image> : Table<"images"> {
    Pk<"id"> images_pk;
    Unique<"url"> images_url_key;
};

template<>
struct flux::reflect::Declaration<ProductImage> : Table<"product_images"> {
    Pk<"product_id", "image_id"> product_images_pk;
    Fk<"image_id", "images", "id", EActionOnDelete::Cascade> product_images_image_fk;
    Default<"replaceable", true> product_images_replaceable;
    Check<"image_id > 0"> product_images_image_check;
};

template<>
struct flux::reflect::Declaration<HandRolled> : Table<"hand_rolled"> {};


ERS_TEST_CASE("schema", "[flux]") {
    using namespace flux::reflect;

    static_assert(table_name<Image> == "images");
    static_assert(Entity<Image>);
    static_assert(!Statement<Image>);

    static_assert(column_count<Image> == 3);
    static_assert(has_field<Image>("cache"));
    static_assert(!has_column<Image>("cache"));
    static_assert(has_column<Image>("path"));

    static_assert(has_identity<Image>);
    static_assert(identity_column<Image> == "id");
    static_assert(!has_identity<ProductImage>);

    static_assert(declaration_is_valid<Image>);
    static_assert(declaration_is_valid<ProductImage>);
}

ERS_TEST_CASE("rows", "[flux]") {
    using namespace flux::reflect;

    static_assert(RowType<Image> && flux::ReadableRow<Image>);
    static_assert(flux::impl::reflect::PlainRow<Plain> && flux::ReadableRow<Plain>);
    static_assert(std::is_same_v<flux::row_reader<Image>::state, ColumnIndex<Image>>);

    static_assert(!RowType<std::string>);
    static_assert(!flux::impl::reflect::PlainRow<std::string>);
    static_assert(flux::ReadableRow<std::string>);

    static_assert(RowType<HandRolled>);
    static_assert(!flux::impl::reflect::MappedRow<HandRolled>);
    static_assert(std::is_empty_v<flux::row_reader<HandRolled>::state>);
}

ERS_TEST_CASE("create_table", "[flux]") {
    constexpr std::string_view images = flux::ddl::create_table<Image>();

    CHECK(contains(images, "CREATE TABLE IF NOT EXISTS images"));
    CHECK(contains(images, "id INTEGER GENERATED ALWAYS AS IDENTITY"));
    CHECK(contains(images, "path VARCHAR(120) NOT NULL"));
    CHECK_FALSE(contains(images, "cache"));
    CHECK(contains(images, "CONSTRAINT images_url_key UNIQUE (url)"));

    constexpr std::string_view product_images = flux::ddl::create_table<ProductImage>();

    CHECK(contains(product_images, "replaceable BOOLEAN NOT NULL DEFAULT TRUE"));
    CHECK(contains(product_images, "CONSTRAINT product_images_pk PRIMARY KEY (product_id, image_id)"));
    CHECK(contains(product_images,
        "CONSTRAINT product_images_image_fk FOREIGN KEY (image_id) "
        "REFERENCES images (id) ON DELETE CASCADE"));
}

ERS_TEST_CASE("custom constraint", "[flux]") {
    constexpr std::string_view t = flux::ddl::create_table<ProductImage>();

    CHECK(contains(t, "CONSTRAINT product_images_image_check CHECK (image_id > 0)"));
}

ERS_TEST_CASE("timestamps", "[flux]") {
    CHECK(sql_of(select_from("images") | where(col("created_at") > ers::clock_t::now()))
        == "SELECT *\nFROM images\nWHERE created_at > $1");
}

#endif
