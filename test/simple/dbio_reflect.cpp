// doctest
#include <doctest/doctest.h>

#ifdef ERSLIB_HAS_REFLECTION

// std
#include <cstdint>
#include <optional>
#include <string>

// ers
#include <erslib/dbio/reflect/ddl.hpp>
#include <erslib/dbio/reflect/entity.hpp>


namespace {
    using namespace dbio::reflect;

    struct Product;
    struct Item;

    struct Money {
        int64_t cents = 0;
    };
}

template<>
struct dbio::reflect::sql_type_traits<Money> {
    static constexpr std::string_view value = "NUMERIC(12, 2)";
};

template<>
struct dbio::Definition<Product> : Table<"products"> {
    [[= Pk()]] uint32_t id;
    std::string title;

    statement::Save save;
};

template<>
struct dbio::Definition<Item> : Table<"items"> {
    [[= Pk()]]
    uint32_t id;

    [[= Fk<Product, "id", OnDelete::Cascade>()]] [[= NotNull()]]
    uint32_t product_id;

    [[= NotNull()]]
    std::string color;

    int32_t size;

    [[= Default<"0">()]]
    int32_t amount;

    [[= Skip()]]
    int cache;

    Unique<"product_id", "color", "size"> sku;

    statement::SelectAll select_all;
    statement::Save save;
    statement::Update update;
    statement::LoadBy<"id"> load_by_id;
    statement::LoadBy<"product_id", "color", "size"> load_by_product_color_size;
};

ERS_DBIO_ENTITY(Product, products_fn, products);
ERS_DBIO_ENTITY(Item, items_fn, items);


TEST_CASE("dbio reflect: definition shape") {
    CHECK(table_name<Item>() == "items");
    CHECK(column_names<Item>().size() == 5);
    CHECK(primary_key<Item>().size() == 1);
    CHECK(std::string_view(primary_key<Item>().front()) == "id");

    // Metadata members and Skip never become columns.
    CHECK(std::string_view(column_names<Item>()[4]) == "amount");
    static_assert(unique_count<Item>() == 1);
    static_assert(statement_count<Item>() == 5);
}

TEST_CASE("dbio reflect: derived roles") {
    constexpr auto save = ^^dbio::Definition<Item>::save;
    constexpr auto upd = ^^dbio::Definition<Item>::update;
    constexpr auto by_sku = ^^dbio::Definition<Item>::load_by_product_color_size;

    CHECK(role_columns<Item, save, Role::In>().size() == 1);
    CHECK(role_columns<Item, save, Role::Out>().size() == 4);

    CHECK(role_columns<Item, upd, Role::Out>().size() == 4);
    CHECK(role_columns<Item, upd, Role::Key>().size() == 1);

    CHECK(role_columns<Item, by_sku, Role::Key>().size() == 3);
    CHECK(role_columns<Item, by_sku, Role::In>().size() == 2);
}

TEST_CASE("dbio reflect: ddl") {
    constexpr std::string_view text = dbio::ddl::create_table<Item>();

    CHECK(text.starts_with("CREATE TABLE IF NOT EXISTS items ("));
    CHECK(text.find("id BIGINT GENERATED ALWAYS AS IDENTITY") != std::string_view::npos);
    CHECK(text.find("amount INTEGER DEFAULT 0") != std::string_view::npos);
    CHECK(text.find("PRIMARY KEY (id)") != std::string_view::npos);
    CHECK(text.find("UNIQUE (product_id, color, size)") != std::string_view::npos);
    CHECK(text.find("FOREIGN KEY (product_id) REFERENCES products (id) ON DELETE CASCADE")
        != std::string_view::npos);
    CHECK(text.find("cache") == std::string_view::npos);
}

TEST_CASE("dbio reflect: sql_type_traits") {
    static_assert(HasSqlType<Money>);
    static_assert(!HasSqlType<void*>);
    static_assert(is_nullable<std::optional<std::string>>());
    static_assert(!is_nullable<std::string>());
}

TEST_CASE("dbio reflect: generated sql") {
    CHECK(sql<Item, ^^dbio::Definition<Item>::select_all>() ==
        "SELECT id, product_id, color, size, amount"
        "\nFROM items");

    CHECK(sql<Item, ^^dbio::Definition<Item>::save>() ==
        "INSERT INTO items (product_id, color, size, amount)"
        "\nVALUES ($1, $2, $3, $4)"
        "\nON CONFLICT (product_id, color, size)"
        "\nDO UPDATE SET amount = excluded.amount"
        "\nRETURNING id");

    CHECK(sql<Item, ^^dbio::Definition<Item>::update>() ==
        "UPDATE items"
        "\nSET product_id = $1, color = $2, size = $3, amount = $4"
        "\nWHERE id = $5");

    CHECK(sql<Item, ^^dbio::Definition<Item>::load_by_id>() ==
        "SELECT product_id, color, size, amount"
        "\nFROM items"
        "\nWHERE id = $1");

    CHECK(sql<Item, ^^dbio::Definition<Item>::load_by_product_color_size>() ==
        "SELECT id, amount"
        "\nFROM items"
        "\nWHERE product_id = $1 AND color = $2 AND size = $3");
}

TEST_CASE("dbio reflect: injected entity and api") {
    Item entity {};
    entity.id = 7;
    entity.product_id = 3;
    entity.color = "red";
    entity.size = 42;
    entity.amount = 5;

    CHECK(params_of<Item, ^^dbio::Definition<Item>::update>(entity).size() == 5);
    CHECK(params_of<Item, ^^dbio::Definition<Item>::save>(entity).size() == 4);
    CHECK(params_of<Item, ^^dbio::Definition<Item>::load_by_id>(entity).size() == 1);

    // The api object carries one callable per statement.
    constexpr size_t item_methods = nonstatic_data_members_of(^^items_fn, std::meta::access_context::current()).size();
    constexpr size_t product_methods = nonstatic_data_members_of(^^products_fn, std::meta::access_context::current()).size();

    static_assert(item_methods == 5);
    static_assert(product_methods == 1);
}

#endif
