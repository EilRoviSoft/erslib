# dbio codegen

PostgreSQL (libpqxx) data-access code generator for the `dbio` runtime.
It reads `*.g.json` descriptors and emits, per descriptor:

- a header (`<name>.g.hpp`) - the entity struct, field/layout enums, `entity_traits` specialization, and the data-access function declarations;
- a source (`<name>.g.cpp`) - the data-access function definitions;
- SQL queries (`<name>/<layout>.sql`) - the same statements as standalone files, for migrations / `CREATE TABLE` (loadable via `dbio::QueryStore::load_directory`).

The generated code targets the `dbio` runtime (`#include <erslib/dbio.hpp>`).

```cpp
auto status = app::user::save(tnx, entity);
auto users  = app::user::select_all(tnx);
```

Each data-access function takes an open `pqxx::dbtransaction&` and returns `ers::Status` (single-row / statement) or `ers::Result<...Generator>` (multi-row). A `dbio::transaction_tag` overload runs the call inside a nested savepoint:

```cpp
auto status = app::user::save(dbio::transaction_tag, tnx, entity);
```

The standalone `.sql` files carry the exact same statement text (rendered once, so they can never drift from the embedded literals) and are emitted for migrations and schema setup; load them via `dbio::QueryStore::load_directory` under labels like `sql.user.save`.

## CLI

```sh
python scripts codegen \
    --dir       <input-dir>     # scanned recursively for *.g.json
    --hpp-dir   <out-include>   # generated headers (mirrors --dir layout)
    --cpp-dir   <out-src>       # generated sources (mirrors --dir layout)
    --query-dir <out-query>     # generated *.sql, grouped by entity name
```

## CMake

`dbio_generate()` is provided by `cmake/dbio.cmake` (included by erslib, and by `find_package(erslib)` for downstream projects).

It works like `protobuf_generate`:

```cmake
dbio_generate(
    TARGET     my_app
    IMPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
    HPP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/generated/include"
    CPP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/generated/src"
    QUERY_DIR  "${CMAKE_CURRENT_BINARY_DIR}/generated/query"
    # OUT_VAR   GENERATED_SOURCES  # optional: receive the .cpp list
)
```

With `TARGET`, the generated sources are added to it, `HPP_DIR` is added to its include path, and it is linked against `dbio`. Generation runs at configure time, so re-run CMake after adding or removing descriptors.

## Descriptor example

```json
{
    "type": "table",
    "namespace": "app",

    "fields": [
        { "type": "BIGINT",  "name": "id", "explicit_type": "uint32_t" },
        { "type": "VARCHAR", "name": "email" }
    ],

    "keys": { "primary": ["id"] },

    "remarks": [
        { "type": "IDENTITY", "field": "id" },
        { "type": "UNIQUE",   "field": "email" }
    ]
}
```

### Field & key options

- **`default`** - either a string or an object `{ "code": "<C++ initializer>", "sql": "<SQL column DEFAULT>" }`. `code` sets the generated struct member's initializer; `sql` emits `DEFAULT <sql>` in the `CREATE TABLE`. A bare string is a **uniform value applied to both** (e.g. `"default": "true"` -> `= true` in C++ and `DEFAULT true` in DDL). Use the object form when they must differ (e.g. `{ "code": "true", "sql": "TRUE" }`) or to set only one side (omit the other key).
- **Foreign keys** - `"keys": { "foreign": { "<col>": { "table": "...", "field": "...", "on_delete": "CASCADE", "on_update": "RESTRICT" } } }`. `on_delete`/`on_update` are optional; the target `table` name is emitted verbatim.
- **Composite keys** - `"keys": { "primary": ["a", "b"] }` and `"remarks": [{ "type": "UNIQUE", "fields": ["a", "b"] }]` emit table-level `PRIMARY KEY (...)` / `UNIQUE (...)`.
- **`table_name`** - overrides the SQL table name (default is the descriptor name pluralized with `s`, e.g. `user` -> `users`).

## Custom table layouts (`select`)

A table descriptor may carry a `layouts` array of hand-written **`select`** queries. Each one returns the table's own entity as an `ers::Result<Generator>` (a `dbio::EntityGenerator<Entity>`), so it reuses the entity struct and `make_config`:

```json
"layouts": [
    {
        "type": "select",
        "name": "select_adults",
        "params": [ { "name": "min_age", "type": "INT" } ],
        "condition": "age >= $1",
        "order_by": "age DESC, name",
        "limit": 100
    },
    {
        "type": "select",
        "name": "select_namesakes",
        "params": [ { "field": "name" } ],
        "condition": "name = $1"
    },
    {
        "type": "select",
        "name": "select_special",
        "params": [ { "name": "min_total", "type": "BIGINT" } ],
        "get": ["id", "name"],
        "sql_file": "sql/select_special.sql"
    }
]
```

- **`name`** - the generated function / layout name; must not collide with the built-in layouts (`select_all`, `save`, `update`, `load_by_*`).
- **`params`** - ordered bind parameters, referenced positionally as `$1..$N`. An entry is either `{ "field": "<column>" }` (reuses that column's C++ type) or `{ "name": "...", "type": "<SQL type>", "explicit_type"?: "..." }` (a free parameter, e.g. a `min_age` that is not a column). They become the function arguments in order.
- **Builder mode** - any of `condition` / `order_by` / `limit` (all optional), emitted verbatim as `WHERE` / `ORDER BY` / `LIMIT`.
- **Raw mode** - `sql` (a string or an array of lines) or `sql_file` (a path relative to the descriptor). Mutually exclusive with builder mode.
- **`get`** - optional subset of columns to select (default: all columns). Always re-ordered to the field declaration order; in raw mode the hand-written `SELECT` list must match that order.

## Standalone queries (`type: queries`)

For queries that are not a projection of a single table (joins, aggregates, bare statements), use a **`queries`** descriptor. It groups any number of queries and emits, per descriptor, a lightweight result `struct` per query plus the data-access functions — no field/layout enums or `entity_traits`.

```json
{
    "type": "queries",
    "namespace": "app",

    "queries": [
        {
            "name": "age_histogram",
            "kind": "rows",
            "params": [ { "name": "min_age", "type": "INT" } ],
            "result": [
                { "name": "age",        "type": "INT" },
                { "name": "user_count", "type": "BIGINT" }
            ],
            "sql": [
                "SELECT age, COUNT(*) AS user_count",
                "FROM users",
                "WHERE age >= $1",
                "GROUP BY age",
                "ORDER BY age"
            ]
        },
        {
            "name": "oldest_user",
            "kind": "row",
            "result": [
                { "name": "id",   "type": "BIGINT", "explicit_type": "uint32_t" },
                { "name": "name", "type": "VARCHAR" }
            ],
            "sql": "SELECT id, name FROM users ORDER BY age DESC LIMIT 1"
        },
        {
            "name": "rename_user",
            "kind": "statement",
            "params": [
                { "name": "id",       "type": "BIGINT", "explicit_type": "uint32_t" },
                { "name": "new_name", "type": "VARCHAR" }
            ],
            "sql_file": "sql/rename_user.sql"
        }
    ]
}
```

- **`kind`** - one of:
  - `rows` - multi-row `SELECT`; returns `ers::Result<<Struct>Generator>` (a `dbio::RowGenerator<Struct>`).
  - `row` - exactly-one-row `SELECT`; returns `ers::Result<Struct>` (throws `pqxx::unexpected_rows` if the query returns other than one row).
  - `statement` - no result rows (`INSERT` / `UPDATE` / `DELETE` / ...); returns `ers::Status`.
- **`params`** - as for `select` layouts, minus field refs (there is no owning table); positional `$1..$N`.
- **`result`** - required for `rows` / `row`, forbidden for `statement`. Each column reuses the field type map (`type`, `explicit_type`, `"flags": ["nullable"]` -> `std::optional<T>`). **Declaration order must match the `SELECT` column order** - columns are read sequentially.
- **`result_name`** - optional struct name (default: the query name in CamelCase, e.g. `age_histogram` -> `AgeHistogram`).
- **`sql` / `sql_file`** - exactly one, same as raw-mode layouts.

The `namespace` places the result structs in `<namespace>` and the functions in `<namespace>::<descriptor-name>` (e.g. `app::AgeHistogram` and `app::stats::age_histogram`). Standalone `.sql` files are emitted under `sql.<descriptor>.<query>` for the query store, exactly like table layouts.

## Errors

An unknown top-level descriptor `type`, an unknown layout `type`, a name collision, a missing `sql_file`, or supplying both `sql` and `sql_file` are hard errors: generation fails and `dbio_generate()` aborts the CMake configure with the offending descriptor path.
