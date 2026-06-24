# dbio codegen

PostgreSQL (libpqxx) data-access code generator for the `erslib::dbio` runtime.
It reads `*.g.json` descriptors and emits, per descriptor:

- a header (`<name>.g.hpp`) — the entity struct, field/layout enums, `entity_traits`
  specialization, the SQL as `inline constexpr std::string_view` constants in a
  per-entity `sql` namespace, and the data-access function declarations;
- a source (`<name>.g.cpp`) — the data-access function definitions;
- SQL queries (`<name>/<layout>.sql`) — the same statements as standalone files,
  for migrations / `CREATE TABLE` (loadable via `dbio::QueryStore::load_directory`).

The generated code targets the `dbio` runtime (`#include <erslib/dbio.hpp>`)
and uses `ers::Result<T>` / `ers::Status` / `ers::Error` for results.

The SQL is **hardcoded into the generated source** — each `_dbio` references the
embedded constant (e.g. `app::user::sql::save`), so the data-access functions take
no query store and there is nothing to load at runtime:

```cpp
auto status = app::user::save(tnx, entity);
auto users  = app::user::select_all(tnx);
// inspect the embedded SQL directly if needed:
static_assert(!app::user::sql::save.empty());
```

The standalone `.sql` files are still emitted (identical text) for migrations and
schema setup; `dbio::QueryStore` remains available for that purpose.

## CLI

```sh
python scripts codegen \
    --dir       <input-dir>     # scanned recursively for *.g.json
    --hpp-dir   <out-include>   # generated headers (mirrors --dir layout)
    --cpp-dir   <out-src>       # generated sources (mirrors --dir layout)
    --query-dir <out-query>     # generated *.sql, grouped by entity name
    [--runtime-namespace ers::dbio]
```

It prints a `;`-separated list of generated `.cpp` files to stdout (consumed by
CMake). `jinja2` is required (`pip install jinja2`).

## CMake

`dbio_generate()` is provided by `cmake/dbio.cmake` (included by erslib, and by
`find_package(erslib)` for downstream projects). It works like
`protobuf_generate`:

```cmake
dbio_generate(
    TARGET     my_app
    IMPORT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src"
    HPP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/generated/include"
    CPP_DIR    "${CMAKE_CURRENT_BINARY_DIR}/generated/src"
    QUERY_DIR  "${CMAKE_CURRENT_BINARY_DIR}/generated/query"
    # NAMESPACE ers::dbio          # optional runtime namespace override
    # OUT_VAR   GENERATED_SOURCES  # optional: receive the .cpp list
)
```

With `TARGET`, the generated sources are added to it, `HPP_DIR` is added to its
include path, and it is linked against `erslib::dbio`. Generation runs at
configure time, so re-run CMake after adding or removing descriptors.

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
