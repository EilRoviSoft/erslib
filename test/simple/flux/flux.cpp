// doctest
#include <doctest/doctest.h>

// std
#include <string>
#include <string_view>

// ers
#include <erslib/flux.hpp>
#include <erslib/flux/query_builder.hpp>


using namespace flux::orm;


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


TEST_CASE("flux: select renders slots in layout order") {
    const auto sql = sql_of(select_from("users")
        | columns("id", "name")
        | where("active", "="_op, true)
        | order_by("created_at", flux::ext::EOrder::Desc)
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

TEST_CASE("flux: select without columns falls back to *") {
    using namespace flux::orm;

    CHECK(sql_of(select_from("users")) == "SELECT *\nFROM users");
}

TEST_CASE("flux: insert with conflict and returning") {
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

TEST_CASE("flux: update and delete") {
    CHECK(sql_of(update("users") | assign("name", std::string("bob")) | where("id", "="_op, 1))
        == "UPDATE users\nSET name = $1\nWHERE id = $2");

    CHECK(sql_of(delete_from("users") | where("id", "="_op, 1))
        == "DELETE FROM users\nWHERE id = $1");
}

TEST_CASE("flux: where variants") {
    CHECK(contains(sql_of(select_from("t") | where_null("a")), "WHERE a IS NULL"));
    CHECK(contains(sql_of(select_from("t") | where_null("a", false)), "WHERE a IS NOT NULL"));
    CHECK(contains(sql_of(select_from("t") | where_in("a", {1, 2, 3})), "WHERE a IN ($1, $2, $3)"));
    CHECK(contains(sql_of(select_from("t") | where("a", "<>"_op, 1)), "WHERE a <> $1"));

    // "x IN ()" is not valid SQL, so an empty list collapses to a constant.
    CHECK(contains(sql_of(select_from("t") | where_in("a", std::vector<int> {})), "WHERE FALSE"));
    CHECK(contains(sql_of(select_from("t") | where_in("a", std::vector<int> {}, true)), "WHERE TRUE"));
}

TEST_CASE("flux: vectors bind as escaped array literals") {
    using flux::impl::sql_value;

    CHECK(sql_value<std::vector<int>>::to_text({ 1, -2, 3 }) == "{1,-2,3}");
    CHECK(sql_value<std::vector<bool>>::to_text({ true, false }) == "{t,f}");
    CHECK(sql_value<std::vector<std::string>>::to_text({}) == "{}");
    CHECK(sql_value<std::vector<std::string>>::to_text({ "a b", "x,y", "q\"t", "b\\s", "", "NULL", "{}" })
        == R"({"a b","x,y","q\"t","b\\s","","NULL","{}"})");
    CHECK(sql_value<std::vector<ers::optional<std::string>>>::to_text({ "a", ers::nullopt }) == R"({"a",NULL})");
}

TEST_CASE("flux: identifiers are validated, values are bound") {
    CHECK_FALSE((select_from("users") | where("id; DROP TABLE users", "="_op, 1)).to_sql().has_value());
    CHECK_FALSE((select_from("users") | columns("1")).to_sql().has_value());
    CHECK_FALSE(select_from("users; --").to_sql().has_value());

    // qualified names stay legal
    CHECK((select_from("public.users") | where("users.id", "="_op, 1)).to_sql().has_value());

    // a hostile value is a parameter, never text
    CHECK(contains(
        sql_of(select_from("t")
            | where("a", "="_op, std::string("'; DROP TABLE t; --"))),
        "WHERE a = $1"
    ));
}

TEST_CASE("flux: a clause outside the layout is rejected") {
    CHECK_FALSE((delete_from("t") | order_by("a")).to_sql().has_value());
}

TEST_CASE("flux: subqueries share the parent placeholder counter") {
    const auto sql = sql_of(select_from("users")
        | columns("id")
        | where("active", "="_op, true)
        | where_exists(select_from("orders") | where("user_id", "="_op, 7))
        | with_limit(10));

    CHECK(contains(sql, "WHERE active = $1 AND EXISTS (SELECT *"));
    CHECK(contains(sql, "WHERE user_id = $2"));
    CHECK(contains(sql, "LIMIT $3"));
}

TEST_CASE("flux: exists_in is a standalone statement") {
    CHECK(sql_of(exists_in("orders") | where("user_id", "="_op, 7))
        == "SELECT EXISTS (\nSELECT 1 FROM orders\nWHERE user_id = $1\n) AS \"exists\"");
}

TEST_CASE("flux: exec() result dispatch") {
    static_assert(flux::ReadableRow<bool>);
    static_assert(flux::impl::RowContainer<std::vector<bool>>);
    static_assert(flux::impl::RowContainer<ers::optional<bool>>);

    static_assert(std::is_same_v<decltype(std::declval<flux::QueryResult>().get_as<bool>()), bool>);
    static_assert(std::is_same_v<
        decltype(std::declval<flux::QueryResult>().get_as<std::vector<bool>>()), std::vector<bool>>);
    static_assert(std::is_same_v<
        decltype(std::declval<flux::QueryResult>().get_as<ers::optional<bool>>()), ers::optional<bool>>);
    static_assert(std::is_same_v<
        decltype(std::declval<flux::QueryResult>().view_as<bool>()), flux::RowGenerator<bool>>);

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

TEST_CASE("flux: with_query accepts anything shaped like exec(tx) -> QueryResult") {
    static_assert(flux::impl::Executable<flux::QueryBuilder>);
    static_assert(flux::impl::Executable<FakeExecutable>);
    static_assert(!flux::impl::Executable<int>);

    static_assert(!flux::impl::ExecutableWithOutput<flux::QueryBuilder>);
    static_assert(!flux::impl::ExecutableWithOutput<FakeExecutable>);
    static_assert(flux::impl::ExecutableWithOutput<FakeQueryCall>);
}


//----------------------------------------------------------------------------------------------------------------------
// reflection
//----------------------------------------------------------------------------------------------------------------------


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


TEST_CASE("flux reflect: schema facts") {
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

TEST_CASE("flux reflect: row concepts") {
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

TEST_CASE("flux reflect: create_table") {
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

TEST_CASE("flux reflect: a constraint kind defined outside the library renders") {
    constexpr std::string_view t = flux::ddl::create_table<ProductImage>();

    CHECK(contains(t, "CONSTRAINT product_images_image_check CHECK (image_id > 0)"));
}

TEST_CASE("flux: timestamps bind as parameters") {
    CHECK(sql_of(select_from("images") | where("created_at", ">"_op, ers::clock_t::now()))
        == "SELECT *\nFROM images\nWHERE created_at > $1");
}

#endif
