# flux reflect

Builds on [flux](../flux/README.md). A table is a struct plus a `Declaration`. From that you get `CREATE TABLE` and reading rows straight into the struct.

Needs `ERSLIB_ENABLE_REFLECTION=ON` and GCC 16.1+. Everything here is behind `ERSLIB_HAS_REFLECTION`.


## Tables

```cpp
struct Image {
    uint32_t id = flux::undefined_id;
    std::string url;
    std::string path;
};

template<>
struct flux::reflect::Declaration<Image> : Table<"images"> {
    Pk<"id"> images_pk;                 // CONSTRAINT images_pk PRIMARY KEY (id)
    Unique<"url"> images_url_key;
    Unique<"path"> images_path_key;
};
```

Member name = constraint name. Postgres keeps index-backed constraints in one namespace per schema, so make them unique across the schema, not just the table.

```cpp
template<>
struct flux::reflect::Declaration<ProductImage> : Table<"product_images"> {
    Pk<"product_id", "image_id"> product_images_pk;
    Unique<"product_id", "position"> product_images_position_key;

    Fk<"product_id", "products", "id", EActionOnDelete::Cascade> product_images_product_fk;   // or SetNull, SetDefault, Restrict
    Fk<"image_id", "images", "id"> product_images_image_fk;

    Check<"position >= 0"> product_images_position_check;
    Default<"replaceable", true> product_images_replaceable;   // name isn't used, call it anything
};
```

- `NOT NULL` unless the field is `ers::maybe`.
- A single integral PK becomes `GENERATED ALWAYS AS IDENTITY`. `Identity<"column">` forces it elsewhere.


## Fields

```cpp
struct Product {
    uint32_t id = flux::undefined_id;

    [[=flux::reflect::Column<"descr">]]                 // other column name
    ers::maybe<std::string> description;

    [[=flux::reflect::Column<"", "VARCHAR(120)">]]      // other SQL type
    std::string title;

    [[=flux::reflect::Skip]]                            // not a column
    int cache;
};
```


## Schemas

```cpp
namespace onec {
    struct Schema : flux::reflect::Schema<"onec"> {};
    struct Price { ... };
}

template<>
struct flux::reflect::Declaration<onec::Price> : Table<"prices", onec::Schema> { ... };
```

Tables of a schema are collected from the namespace the schema type is in (nested ones too). A table declared outside of it won't be found. Without a schema the name stays unqualified.


## DDL

```cpp
constexpr std::string_view ddl = flux::ddl::create_table<Image>();
```

Built at compile time:

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


## Reading rows

```cpp
auto collections = flux::with_tx(db.pool(), [limit](pqxx::read_transaction& tx) {
    using namespace flux::orm;

    return (select_from("collections")
        | order_by_random()
        | with_limit(limit)
    ).exec(tx).get_as<std::vector<Collection>>();
}, "Collection/SelectRandomly");
```

```cpp
result.get_as<Image>()                      // exactly one row, throws otherwise
result.get_as<ers::maybe<Image>>()       // 0..1
result.get_as<std::vector<Image>>()         // 0..N
result.view_as<Image>()                     // lazy RowGenerator, 0..N
```

Columns are matched by name, so `SELECT *` works, and missing columns keep the struct's defaults:

```cpp
auto inserted = (insert_into("images")
    | columns("url", "path")
    | values(img.url, img.path)
    | returning("id")
).exec(tx).get_as<Image>();     // only 'id' is set
```


## Writing rows

```cpp
template<>
struct flux::reflect::Declaration<User> : Table<"users"> {
    Pk<"id"> users_pk;
    Unique<"email"> users_email_key;
};

template<>
struct flux::reflect::DeclarationMetadata<User> {       // optional: how the code uses the table, not the schema
    ReadOnly<"created_at"> users_created_ro;            // the database fills it: not in INSERT/UPDATE, read back
};
```

```cpp
Image saved = flux::crud::insert(tx, img);                  // RETURNING *, so id and defaults are in it
ers::maybe<Image> found = flux::crud::find<Image>(tx, 42); // composite key: find<ProductImage>(tx, product_id, image_id)
bool updated = flux::crud::update(tx, saved);               // false if no row matched
bool erased = flux::crud::erase<Image>(tx, 42);
Image merged = flux::crud::upsert<"images_url_key">(tx, img);

flux::crud::find<Image>(tx, "42");                          // doesn't compile: the key is a uint32_t
```

The key types are checked against the primary key fields. `insert` leaves out the identity and `ReadOnly` columns, `update` also the key.

```cpp
flux::crud::upsert(link);                                   // ON CONFLICT (product_id, image_id), by the primary key
flux::crud::upsert<"images_url_key">(img);                  // ON CONFLICT ON CONSTRAINT, a Pk/Unique name of the declaration
```

An entity with a generated key needs the constraint name.

```cpp
constexpr auto cols = flux::columns_of<Image>;

flux::crud::update(tx, img, flux::crud::only(cols.path));           // SET path = $1, checked while compiling
flux::crud::update(tx, img, flux::crud::only("path", "url"));       // by name, a bad one fails when rendered

flux::crud::find<Image, Preview>(tx, 42);                           // a projection: only the columns of Preview
```

The builders return a `QueryBuilder`, everything from [flux](../flux/README.md) works on them:

```cpp
flux::crud::select<Image>() | where(cols.url == url) | with_limit(1);   // SELECT id, url, path FROM images ...
flux::crud::insert(img) | returning("id");
flux::crud::erase<Image>(42);                                           // DELETE FROM images WHERE id = $1
flux::crud::where_pk<Image>(42);                                        // just the condition
```


## Hidden conditions

```cpp
template<>
struct flux::reflect::DeclarationMetadata<Note> {
    SoftDelete<"deleted_at"> notes_soft;            // nullable: NULL while the row lives, the time of the deletion after it

    static void scope(flux::QueryBuilder& q) {      // optional: for every select/update/erase of Note
        q |= where(col("tenant") == current_tenant());
    }
};
```

```cpp
flux::crud::find<Note>(tx, 42);               // ... AND deleted_at IS NULL, a deleted note is "not found"
flux::crud::erase<Note>(tx, 42);              // UPDATE notes SET deleted_at = now() WHERE id = $1 AND ... AND deleted_at IS NULL
flux::crud::update(tx, note);                 // skips the deleted rows, never writes deleted_at
flux::crud::restore<Note>(tx, 42);            // SET deleted_at = NULL

flux::crud::select_all<Note>();               // with the deleted rows
flux::crud::find_all<Note>(tx, 42);
flux::crud::erase_forever<Note>(tx, 42);      // a real DELETE
```

`scope` runs on every query, so it can read the current tenant from the thread or the request. The `_all` and `_forever` forms skip only SoftDelete, the scope still applies. `insert` and `upsert` are never changed, an `upsert` on a unique key can update a deleted row.


## Typed columns

```cpp
constexpr auto img = flux::columns_of<Image>;       // one member per column, named like the fields

where(img.url == url)                               // url = $1
order_by(img.id, EOrder::Desc)
columns(img.id, img.url.as("link"))
img.id == "42"                                      // doesn't compile

auto i = flux::columns_as<Image>("i");              // qualified, the alias has to outlive it
auto p = flux::columns_as<Product>("p");
select_from("images", "i") | join("products", "p", i.id == p.image_id);     // i.id = p.image_id
```

Your own type works the same way if it has `Expr to_expr() const`:

```cpp
struct Money : flux::ext::ExprMethods {               // is_null(), in(), as(), ... for free
    flux::Expr to_expr() const;

    template<typename U>                                    // optional: what it may be compared with
    static constexpr bool accepts = flux::ext::IsExpr<U> || std::convertible_to<U, long>;
};
```


## Rows that aren't tables

A plain aggregate needs no `Declaration`:

```cpp
struct CascadeHit {
    std::string source_table;
    uint32_t id;
    uint32_t image_id;
};

auto hits = query.exec(tx).get_as<std::vector<CascadeHit>>();
```

`RowType` = has a `Declaration`, `Entity` = also has a `Table`. So `create_table<CascadeHit>()` doesn't compile.

A type constructible from `pqxx::row_ref` is read through that constructor instead.


## SQL files

```cpp
ERS_DECLARE_QUERY(onec::queries, record_run, "onec.record_run") {
    struct Input {          // binds as $1, $2, ... in this order
        std::string job;
        i64 elapsed_ms;
        bool ok;
    };

    // using Output = ...;  // with_query returns this instead of QueryResult
    // struct Fragments { std::string sort; };  // replaces {{sort}} as text, never user input
};

flux::with_query(db.pool(), "Onec/RecordRun", onec::queries::record_run({
    .job        = "prices",
    .elapsed_ms = 42,
    .ok         = true,
}));
```

`"onec.record_run"` is `onec/record_run.sql` in the folder loaded with `flux::queries.load_directory()`.


## Migrations

```cpp
auto migrated = flux::migrate<core::Schema, onec::Schema>(db.pool(), root / "migrations");
```

```
0001_name.existing.sql    // only on a database that already existed, a fresh one just marks it applied
0002_name.fresh.sql       // only on a fresh database
0003_name.always.sql      // both
name.repeatable.sql       // every run
```

If a declaration changed, flux stops until an `.existing.sql` migration says what happened to the table:

```sql
-- flux: table public.images 1a2b3c4d5e6f7a8b
-- flux: drop public.old_images
```

The error message contains the exact line to paste.


## What the declaration knows

```cpp
flux::reflect::table_name<Image>                        // "images"
flux::reflect::primary_key<Image>                       // span of column names
flux::reflect::pk_constraint<Image>                     // "images_pk"
flux::reflect::identity_column<Image>                   // "id", empty if none
flux::reflect::has_identity<Image>
flux::reflect::column_count<Product>
flux::reflect::column_name<^^Product::description>      // "descr"
flux::reflect::sql_type_name<^^Product::title>          // "VARCHAR(120)"
flux::reflect::is_nullable<^^Product::description>      // true
flux::reflect::is_pk_column<Image, ^^Image::id>         // true
flux::reflect::is_identity_column<Image, ^^Image::id>   // true
```

```cpp
auto saved = (insert_into("images")
    | columns("url", "path")
    | values(img.url, img.path)
    | on_conflict_constraint("images_url_key")
    | do_update("path")
    | returning(std::string(flux::reflect::identity_column<Image>))
).exec(tx).get_as<Image>();
```


## Your own types

```cpp
template<>
struct flux::sql_type<Money> {
    static constexpr std::string_view name = "NUMERIC(12, 2)";
    static constexpr bool nullable = false;
};

template<>
struct flux::sql_value<Money> {
    static std::string to_text(const Money& what) {         // used for arrays
        return what.to_string();
    }

    static void bind(pqxx::params& out, const Money& what) {
        out.append(to_text(what));
    }

    static Money read(const auto& field) {
        return Money(field.template as<std::string>());
    }
};
```

For a mapping without reflection, specialize `flux::row_reader<T>`. That's what `get_as` and `view_as` go through.


## Your own constraints

`Check` is just this, and yours can be the same:

```cpp
template<ers::fixed_string Expr>
struct Check {
    using kind = kinds::ECheck;
};

template<ers::fixed_string Expr>
struct entry_traits<Check<Expr>> : entry_defaults {
    static consteval std::string ddl(const constraint_context& ctx) {
        return "CONSTRAINT " + std::string(ctx.name) + " CHECK (" + std::string(Expr.to_sv()) + ")";
    }
};
```
