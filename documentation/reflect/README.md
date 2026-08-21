# dbio reflect

You can describe a table once, as a struct + a `Declaration` specialization. The `CREATE TABLE` statement and the ability to read query results straight into that struct are the outputs of this.

Needs `ERSLIB_ENABLE_REFLECTION=ON` and GCC 16.1 or newer. Everything below sits behind `ERSLIB_HAS_REFLECTION`.


## Declaring a table

```cpp
struct Image {
    uint32_t id = dbio::undefined_id;
    std::string url;
    std::string path;
};

template<>
struct dbio::reflect::Declaration<Image> : Table<"images"> {
    Pk<"id"> images_pk;
    Unique<"url"> images_url_key;
    Unique<"path"> images_path_key;
};
```

Each constraint is a member, and the member's name becomes the constraint's name in SQL, so `Pk<"id"> images_pk` gives you `CONSTRAINT images_pk PRIMARY KEY (id)`.

The template arguments are column names. Keep the member names unique across the whole schema and table: Postgres puts index-backed constraints in one namespace per schema, so a second `id_pk` somewhere else will fail at `CREATE TABLE` time with a name clash.

## Rest of the vocabulary

```cpp
template<>
struct dbio::reflect::Declaration<ProductImage> : Table<"product_images"> {
    Pk<"product_id", "image_id"> product_images_pk;
    Unique<"product_id", "position"> product_images_position_key;

    Fk<"product_id", "products", "id", action_on_delete::cascade> product_images_product_fk;
    Fk<"image_id", "images", "id"> product_images_image_fk;

    Default<"replaceable", true> product_images_replaceable;
};
```

`Fk` takes the local column, the referenced table and column, and optionally one of `action_on_delete::cascade`, `set_null`, `set_default` or `restrict`. `Default` takes a column and a value, and unlike the constraints it does not use its member name for anything, so call it whatever
reads well.

Columns are `NOT NULL` unless the field is an `ers::optional`. A lone integral primary key is treated as `GENERATED ALWAYS AS IDENTITY` automatically; `Identity<"column">` exists for when you want to force that on some other column.


## Per-field tweaks

Renaming a column, overriding its SQL type, or leaving a field out of the table are all facts about one field, so they go on the field:

```cpp
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
```


## Getting the DDL

```cpp
constexpr std::string_view ddl = dbio::ddl::create_table<Image>();
```

It is the only `consteval` DDL, so the text is built during compilation and costs nothing at runtime (except actually running it):

```sql
CREATE TABLE IF NOT EXISTS images (
    id INTEGER GENERATED ALWAYS AS IDENTITY,
    url TEXT NOT NULL,
    path TEXT NOT NULL,
    CONSTRAINT images_pk PRIMARY KEY (id),
    CONSTRAINT images_url_key UNIQUE (url),
    CONSTRAINT images_path_key UNIQUE (path)
)
```


## Reading results

Include `reflect.hpp` and `exec_as` / `exec_one` start accepting your entities:

```cpp
auto collections = db.with_transaction([limit](pqxx::read_transaction& tx) {
    ERS_QUICK_DBIO_USING;
    return (select_from("collections")
        | order_by_random()
        | with_limit(limit)
    ).exec_as<Collection>(tx);
}, "Collection/SelectRandomly");
```

`exec_as<T>` hands back `ers::Result<RowGenerator<T>>` - a lazy range you can iterate or ask for a size. `exec_one<T>` hands back `ers::Result<T>` and fails unless the query returned exactly one row.

Columns are matched by name rather than position, which has two consequences worth knowing. `SELECT *` works. And a query that returns only some of the columns fills only those, leaving the rest at the struct's defaults, so the call below gives you an `Image` with `id` set and `url` and `path` still empty:

```cpp
auto inserted = (insert_into("images")
    | columns("url", "path")
    | values(img.url, img.path)
    | returning("id")
).exec_one<Image>(tx);
```


## Reusing what the declaration knows

Queries you write by hand do not have to repeat names you have already written down:

```cpp
dbio::reflect::table_name<Image>                        // "images"
dbio::reflect::primary_key<Image>                       // span of column names
dbio::reflect::pk_constraint<Image>                     // "images_pk"
dbio::reflect::identity_column<Image>                   // "id", empty when there is none
dbio::reflect::has_identity<Image>
dbio::reflect::column_count<Product>
dbio::reflect::column_name<^^Product::description>      // "descr"
dbio::reflect::sql_type_name<^^Product::title>          // "VARCHAR(120)"
dbio::reflect::is_nullable<^^Product::description>      // true
dbio::reflect::is_pk_column<Image, ^^Image::id>         // true
dbio::reflect::is_identity_column<Image, ^^Image::id>   // true
```

```cpp
auto saved = (insert_into("images")
    | columns("url", "path")
    | values(img.url, img.path)
    | on_conflict_constraint("images_url_key")
    | do_update("path")
    | returning(std::string(dbio::reflect::identity_column<Image>))
).exec_one<Image>(tx);
```

## Rows that are not tables

A join or an aggregate does not come from a single table, but you still want it read into a struct. Give it a `Declaration` with no `Table` base:

```cpp
struct CascadeHit {
    std::string source_table;
    uint32_t id;
    uint32_t image_id;
};

template<>
struct dbio::reflect::Declaration<CascadeHit> {};
```

`exec_as<CascadeHit>` now works, and `create_table<CascadeHit>()` does not compile, which is the point. The two concepts are `RowType`, meaning there is a `Declaration` and rows can be read into it, and `Entity`, meaning there is also a `Table` and DDL can be generated.


## Types it does not know yet

Two traits, specialized the usual way. `sql_type` says what a C++ type looks like in DDL:

```cpp
template<>
struct dbio::reflect::sql_type<Money> {
    static constexpr std::string_view name = "NUMERIC(12, 2)";
    static constexpr bool nullable = false;
};
```

`sql_value` says how a value crosses into a query and back out of a result:

```cpp
template<>
struct dbio::reflect::sql_value<Money> {
    static void bind(pqxx::params& out, const Money& what) {
        out.append(what.to_string());
    }

    static Money read(const auto& field) {
        return Money(field.template as<std::string>());
    }
};
```

If you need a mapping that has nothing to do with reflection, `row_reader<T>` is the hook `exec_as` actually dispatches through, and you can specialize it directly.
