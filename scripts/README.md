# dbio codegen

PostgreSQL (libpqxx) data-access code generator for the `dbio` runtime.
It reads `*.g.json` descriptors and emits, per descriptor:

- a header (`<name>.g.hpp`) - the entity struct, field/layout enums, `entity_traits` specialization, and the data-access function declarations;
- a source (`<name>.g.cpp`) - the data-access function definitions;
- SQL queries (`<name>/<layout>.g.sql`) - the same statements as standalone files, for migrations / `CREATE TABLE` (loadable via `dbio::QueryStore::load_directory`).

`--dir`/`IMPORT_DIR` is scanned recursively for every `*.g.json` descriptor, `table`, `enum`, and `queries` alike - there's no restriction on where a given descriptor type has to live, they can all sit side by side in the same tree. `--query-dir`/`QUERY_DIR` is purely an output directory (like `--hpp-dir`/`--cpp-dir`): every descriptor's generated SQL lands there, grouped by descriptor name.

Generated SQL files always use the **`.g.sql`** extension, never plain `.sql`, so they're never mistaken for hand-authored `.sql` files (e.g. `sql_file` references) that might live nearby. `dbio::QueryStore::load_directory` only loads `*.g.sql` files for the same reason.

The generated code targets the `dbio` runtime (`#include <erslib/dbio.hpp>`).

```cpp
auto status = app::user::save(tnx, entity);
auto users  = app::user::select_all(tnx);
```

Each data-access function takes an open `pqxx::dbtransaction&` and returns `ers::Status` (single-row / statement) or `ers::Result<...Generator>` (multi-row). A `dbio::transaction_tag` overload runs the call inside a nested savepoint:

```cpp
auto status = app::user::save(dbio::transaction_tag, tnx, entity);
```

The standalone `.g.sql` files carry the exact same statement text (rendered once, so they can never drift from the embedded literals) and are emitted for migrations and schema setup; load them via `dbio::QueryStore::load_directory` under labels like `sql.user.save`.

## CLI

```sh
python scripts codegen \
    --dir               <input-dir>     # scanned recursively for *.g.json (table/enum/queries alike)
    --hpp-dir           <out-include>   # generated headers (mirrors --dir layout)
    --cpp-dir           <out-src>       # generated sources (mirrors --dir layout)
    --query-dir         <out-query>     # generated *.g.sql, grouped by descriptor name (output only)
    --runtime-namespace <ns>            # dbio runtime namespace (default: dbio)
    --use-query-store                   # generate dbio::queries lookups instead of embedded SQL literals (see below)
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

`IMPORT_DIR` is scanned for every descriptor type - `table`, `enum`, and `queries` can all live under it, mixed together. `QUERY_DIR` is output-only and can safely be a build directory; it receives every descriptor's generated SQL (as `*.g.sql`), grouped by descriptor name.

Pass `USE_QUERY_STORE` to switch the generated code from embedded SQL literals to `dbio::QueryStore` lookups (see below); it requires `erslib::dbio` to have been built with `ERSLIB_DBIO_OWN_QUERY_STORE=ON`, otherwise `dbio_generate()` fails at configure time.

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

The `namespace` places the result structs in `<namespace>` and the functions in `<namespace>::<descriptor-name>` (e.g. `app::AgeHistogram` and `app::stats::age_histogram`). Standalone `.g.sql` files are emitted under `sql.<descriptor>.<query>` for the query store, exactly like table layouts.

## Embedded SQL vs. `dbio::QueryStore` (`--use-query-store`)

By default every generated data-access function embeds its SQL as a `static constexpr std::string_view` literal in the `.cpp` file. Passing **`--use-query-store`** to the generator changes what gets emitted: the literal is omitted entirely and the function instead reads its SQL from the global `dbio::queries` store, by the same label the standalone `.g.sql` files use (e.g. `sql.user.save`, `sql.stats.age_histogram`):

```cpp
// --use-query-store off (default)
static constexpr std::string_view sql_save = R"__sql(...)__sql";
...
    return tnx.exec(sql_save, pqxx::params { ... });
```
```cpp
// --use-query-store on
...
    const std::string_view sql_save = dbio::queries["sql.user.save"];
    return tnx.exec(sql_save, pqxx::params { ... });
```

This is a **codegen-time** decision, not a compile-time `#ifdef` - the generator renders exactly one of the two forms into the `.cpp`, so a given generated tree always uses one mode. Regenerate (with or without the flag) to switch a target. The simplest way to turn it on is the `USE_QUERY_STORE` flag on `dbio_generate()`:

```cmake
dbio_generate(
    TARGET     my_app
    IMPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
    HPP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/generated/include"
    CPP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/generated/src"
    QUERY_DIR  "${CMAKE_CURRENT_BINARY_DIR}/res/query"
    USE_QUERY_STORE
)
```

which just passes `--use-query-store` through to the generator.

**Prerequisites**, since `dbio::queries` is itself conditionally compiled:

- `erslib::dbio` must be built with `-DERSLIB_DBIO_OWN_QUERY_STORE=ON`, which makes the library define `ERS_DBIO_GLOBAL_QUERY_STORE` as a `PUBLIC` compile definition (so every consumer sees `extern dbio::QueryStore queries;`) and, together with `ERS_DBIO_GLOBAL_QUERY_STORE_INIT`, auto-populates it from `./res/query` (relative to the process's working directory) at static-init time. `dbio_generate(... USE_QUERY_STORE ...)` checks this option and fails the configure early if it's off.
- If your `QUERY_DIR` differs from `./res/query` (it usually does), the auto-populated store won't have your labels. Load your own directory once at startup, before any generated function runs:
  ```cpp
  dbio::queries.load_directory(query_dir);
  ```
- Looking up a label that isn't loaded is undefined behavior (`QueryStore::operator[]` does not bounds-check) - make sure every `QUERY_DIR` a `USE_QUERY_STORE`-enabled target generates into gets loaded before use.

## Errors

An unknown top-level descriptor `type`, an unknown layout `type`, a name collision, a missing `sql_file`, or supplying both `sql` and `sql_file` are hard errors: generation fails and `dbio_generate()` aborts the CMake configure with the offending descriptor path.
