// doctest
#include <doctest/doctest.h>

#ifdef ERSLIB_HAS_REFLECTION

// std
#include <cstdint>
#include <optional>
#include <string>

// ers
#include <erslib/dbio/reflect.hpp>


using namespace dbio::reflect;


namespace {
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
struct dbio::reflect::Definition<Product> : Table<"products"> {
    [[= Pk()]]
    uint32_t id;
    std::string title;
};

template<>
struct dbio::reflect::Definition<Item> : Table<"items"> {
    [[= Pk()]]
    uint32_t id;

    [[= Fk<Product, "id", OnDelete::Cascade>()]]
    uint32_t product_id;

    std::string color;

    int32_t size;

    [[= Default<"0">()]]
    int32_t amount;

    [[= Nullable()]]
    std::string note;

    [[= Skip()]]
    int cache;

    Unique<"product_id", "color", "size"> sku;
};

namespace {
    ERS_DBIO_ENTITY(Product);
    ERS_DBIO_ENTITY(Item);
}


TEST_CASE("dbio reflect: definition shape") {
    CHECK(table_name<Item>() == "items");
    CHECK(column_names<Item>().size() == 6);
    CHECK(primary_key<Item>().size() == 1);
    CHECK(std::string_view(primary_key<Item>().front()) == "id");
    CHECK(std::string_view(column_names<Item>()[4]) == "amount");
    CHECK(std::string_view(column_names<Item>()[5]) == "note");
    static_assert(unique_count<Item>() == 1);
}

TEST_CASE("dbio reflect: ddl") {
    constexpr std::string_view text = dbio::ddl::create_table<Item>();

    CHECK(text.starts_with("CREATE TABLE IF NOT EXISTS items ("));
    CHECK(text.find("id BIGINT GENERATED ALWAYS AS IDENTITY") != std::string_view::npos);
    CHECK(text.find("amount INTEGER NOT NULL DEFAULT 0") != std::string_view::npos);
    CHECK(text.find("note TEXT,") != std::string_view::npos);
    CHECK(text.find("color TEXT NOT NULL") != std::string_view::npos);
    CHECK(text.find("PRIMARY KEY (id)") != std::string_view::npos);
    CHECK(text.find("UNIQUE (product_id, color, size)") != std::string_view::npos);
    CHECK(text.find("FOREIGN KEY (product_id) REFERENCES products (id) ON DELETE CASCADE") != std::string_view::npos);
    CHECK(text.find("cache") == std::string_view::npos);
}

TEST_CASE("dbio reflect: sql_type_traits") {
    static_assert(HasSqlType<Money>);
    static_assert(!HasSqlType<void*>);
    static_assert(is_nullable<std::optional<std::string>>());
    static_assert(!is_nullable<std::string>());
}

TEST_CASE("dbio reflect: injected entity") {
    Item entity {};
    entity.id = 7;
    entity.product_id = 3;
    entity.color = "red";
    entity.size = 42;
    entity.amount = 5;
    entity.note = "n";

    constexpr size_t members = nonstatic_data_members_of(^^Item, std::meta::access_context::current()).size();
    static_assert(members == 6);
}

#endif
