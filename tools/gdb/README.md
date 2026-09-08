# Boost GDB pretty printers

GDB pretty printers for Boost containers that GDB and libstdc++ know nothing
about. Written against **Boost 1.91**, verified with GDB 17.1 / Python 3.14.

The directory is self-contained and has no erslib dependency — copy
`tools/gdb/` into any project, or point several projects at one checkout.

## Coverage

| Family | Types |
| --- | --- |
| Boost.Container | `vector`, `small_vector`, `static_vector`, `flat_set`, `flat_multiset`, `flat_map`, `flat_multimap`, `vec_iterator` |
| Boost.Unordered — open addressing | `unordered_flat_map`, `unordered_flat_set`, `unordered_node_map`, `unordered_node_set`, `foa::table_iterator` |
| Boost.Unordered — closed addressing | `unordered_map`, `unordered_multimap`, `unordered_set`, `unordered_multiset`, and their iterators |
| Boost.MultiIndex | `multi_index_container`, the `ordered` / `hashed` / `sequenced` / `random_access` index views returned by `get<Tag>()`, and all index iterators |

`multi_index_container` prints in index #0 order — the order its own `begin()`
uses. An index view prints in that index's order, so
`const auto& idx = mi.get<by_name>();` gives you the hashed ordering.

Flat containers work over any underlying sequence, including one that is not a
Boost container: if the sequence is not a `boost::container::vector` the
printers defer to whatever printer GDB already has for it.

The concurrent containers (`concurrent_flat_map` and friends) are **not**
covered.

## Enabling

Add one line to `~/.gdbinit`:

```bash
echo "source /path/to/tools/gdb/boost-gdb-printers.py" >> ~/.gdbinit
```

For Visual Studio's WSL/Linux debugging, add it to `.vs/launch.vs.json`
instead, next to the `-enable-pretty-printing` entry that is already there:

```json
{
  "text": "source /home/you/project/tools/gdb/boost-gdb-printers.py",
  "ignoreFailures": false
}
```

VS Code's `cppdbg` takes the same `setupCommands` shape. From a Python
context (another printer bundle, a `-gdb.py` sidecar) use:

```python
import boost_printers
boost_printers.register()
```

## Interaction with Boost's own printers

Since 1.87 Boost.Unordered embeds a GDB script into every object file that
includes it (`.debug_gdb_scripts`, see
`boost/unordered/detail/unordered_printers.hpp`). Those are *objfile* printers
and GDB consults objfile printers before global ones, so **Boost's own
implementation wins wherever it is present** — that is deliberate, and the
reason these printers register globally rather than per objfile.

The Boost.Unordered printers here therefore only take effect for builds that
define `BOOST_ALL_NO_EMBEDDED_GDB_SCRIPTS`, for objects whose
`.debug_gdb_scripts` section was stripped, and for Boost older than 1.87.
Nothing is shadowed for Boost.Container or Boost.MultiIndex, which ship no
printers of their own.

`info pretty-printer` shows which is active.

## Limitations

- **Fancy pointers.** Allocators whose `pointer` is a class type are unwrapped
  by taking the single raw pointer inside them, which covers the common
  idioms but not `boost::interprocess::offset_ptr` (its value is an offset,
  not an address).
- **End iterators.** An iterator printer dereferences whatever it points at.
  For a past-the-end iterator that is the header node or one-past the storage,
  so it prints garbage — the same behaviour as libstdc++'s `std::vector`
  iterator printer.
- **`node_count` bounds the walk.** A multi_index container whose links are
  corrupt prints at most `node_count` elements instead of looping forever.
- **Virtual bases are skipped** when resolving base subobject offsets. No
  container handled here uses virtual inheritance.

A container whose layout does not match yields a single `<error: ...>` child
rather than a Python traceback, so a half-constructed object in a locals
window stays harmless.

## Layout assumptions

The printers read Boost's internals directly, so a Boost upgrade can break
them. The load-bearing details, all verified against 1.91:

- `boost::container::vector` keeps `m_start` / `m_size` / `m_capacity` in
  `m_holder`. Version-0 allocators (how `static_vector` is built) drop
  `m_start` and hold the elements in the allocator's own `storage` buffer.
- `flat_set`/`flat_multiset` derive from `dtl::flat_tree`, while
  `flat_map`/`flat_multimap` hold one as the `m_flat_tree` member; either way
  the elements are `m_data.m_seq`.
- `foa::table_core` stores elements at `elements_[group * 15 + slot]`, with a
  slot occupied iff its metadata byte is non-zero. The last group reserves
  slot 14 for a sentinel. Both the SSE2 byte layout and the portable
  bit-interleaved layout are handled.
- Closed-addressing tables walk `buckets_.buckets[i]` and follow each
  null-terminated node chain.
- MultiIndex nodes stack one `*_index_node_impl` base per index; links point at
  those base subobjects, so converting back to a node subtracts the base
  offset. The value lives in the `space` buffer of `pod_value_holder`. The
  hashed traversal is a port of `hashed_index_node_alg::after` from
  `hash_index_node.hpp` for both the unique and non-unique algorithms.

## Running the self-test

`test/self_test.cpp` builds a fixture holding every supported container shape
(empty, inline-storage, heap, custom comparators, alternative sequences) and
`test/self_test.py` asserts the printers reproduce the expected contents. It
exits non-zero on failure.

```bash
BOOST_INC=/path/to/boost/include
TOOLS=/path/to/tools/gdb
g++ -std=c++20 -g -O0 -DBOOST_ALL_NO_EMBEDDED_GDB_SCRIPTS -I"$BOOST_INC" \
    "$TOOLS/test/self_test.cpp" -o /tmp/self_test
LINE=$(grep -n BREAKPOINT "$TOOLS/test/self_test.cpp" | cut -d: -f1)
gdb -batch -ex "set print elements unlimited" -ex "b self_test.cpp:$LINE" \
    -ex run -ex "source $TOOLS/test/self_test.py" /tmp/self_test
```

`-DBOOST_ALL_NO_EMBEDDED_GDB_SCRIPTS` matters: without it Boost's embedded
printers take over the unordered containers, and the script reports those
checks as skipped rather than exercising this code.
