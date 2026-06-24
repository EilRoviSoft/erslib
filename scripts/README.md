# dbio codegen

PostgreSQL (libpqxx) data-access code generator for the `erslib::dbio` runtime.
It reads `*.g.json` descriptors and emits, per descriptor:

- a header (`<name>.g.hpp`) — the entity struct, field/layout enums, `entity_traits`
  specialization and the data-access function declarations;
- a source (`<name>.g.cpp`) — the data-access function definitions;
- SQL queries (`<name>/<layout>.sql`) — registered at runtime by label
  `sql.<name>.<layout>` (see `dbio::QueryStore::load_directory`).

The generated code targets the `dbio` runtime (`#include <erslib/dbio.hpp>`)
and uses `ers::Result<T>` / `ers::Status` / `ers::Error` for results.

Each generated data-access function takes a `const dbio::QueryStore&` as its first
argument (after the optional transaction tag) — there is no global query store, so
you construct/populate one and pass it explicitly:

```cpp
dbio::QueryStore queries;
queries.load_directory(res_dir / "query");
// ...
auto status = app::user::save(queries, tnx, entity);
```

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
