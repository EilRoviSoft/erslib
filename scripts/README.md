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
// inspect the embedded SQL directly if needed:
static_assert(!app::user::sql::save.empty());
```

The standalone `.sql` files are still emitted (identical text) for migrations and schema setup; `dbio::QueryStore` remains available for that purpose.

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
