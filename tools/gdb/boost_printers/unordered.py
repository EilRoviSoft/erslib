"""Pretty printers for Boost.Unordered.

Two unrelated implementations live behind the ``boost::unordered`` namespace:

* **Open addressing** (``unordered_flat_map``/``_set``, ``unordered_node_map``/
  ``_set``) -- ``detail::foa::table_core`` holds ``arrays`` (a metadata group
  array plus a parallel element array) and ``size_ctrl``. Elements are stored
  at ``elements_[group * 15 + slot]``; a slot is occupied iff its metadata byte
  is non-zero. The last group reserves slot 14 for a sentinel.
* **Closed addressing** (``unordered_map``/``_set``/``_multimap``/
  ``_multiset``) -- ``detail::table`` holds a ``grouped_bucket_array``; each
  bucket points at a null-terminated chain of nodes.

Note that Boost >= 1.87 embeds its own GDB printers for these containers into
every translation unit that includes them (``.debug_gdb_scripts``). Those are
objfile printers and GDB consults objfile printers before global ones, so the
Boost implementation wins whenever it is present. These exist for builds that
define ``BOOST_ALL_NO_EMBEDDED_GDB_SCRIPTS``, for stripped objects, and for
older Boost releases.
"""

import gdb

from ._util import (
    GdbError,
    find_base,
    print_elements_limit,
    raw_pointer,
    read_bytes,
    safe_children,
    strip_type,
    template_name,
)

FOA_TABLE_CORE = "boost::unordered::detail::foa::table_core"
FOA_ELEMENT_TYPE = "boost::unordered::detail::foa::element_type"
FOA_ITERATOR = "boost::unordered::detail::foa::table_iterator"
FCA_TABLE = "boost::unordered::detail::table"
FCA_ITERATORS = (
    "boost::unordered::detail::iterator_detail::iterator",
    "boost::unordered::detail::iterator_detail::c_iterator",
)

FOA_MAPS = ("boost::unordered::unordered_flat_map", "boost::unordered::unordered_node_map")
FOA_SETS = ("boost::unordered::unordered_flat_set", "boost::unordered::unordered_node_set")
FCA_MAPS = ("boost::unordered::unordered_map", "boost::unordered::unordered_multimap")
FCA_SETS = ("boost::unordered::unordered_set", "boost::unordered::unordered_multiset")

# group15 geometry: 15 usable slots in a 16-byte metadata group.
GROUP_SLOTS = 15
GROUP_BYTES = 16

_OCCUPIED_MASK = 0x0001000100010001
_OVERFLOW_MASK = 0x4000400040004000


def _unwrap_element(element):
    """Node containers store a pointer to the value; flat ones store it inline."""
    if template_name(element.type) == FOA_ELEMENT_TYPE:
        return raw_pointer(element["p"]).dereference()
    return element


def foa_core(val):
    table = val["table_"]
    found = find_base(table.type, FOA_TABLE_CORE)
    return table.cast(found[1]) if found else table


def foa_size(val):
    return int(foa_core(val)["size_ctrl"]["size"])


def foa_elements(val):
    """Yield the live elements of an open-addressing table."""
    core = foa_core(val)
    arrays = core["arrays"]
    elements = raw_pointer(arrays["elements_"])
    if int(elements) == 0:
        return
    groups = raw_pointer(arrays["groups_"])
    group_count = int(arrays["groups_size_mask"]) + 1
    group_type = strip_type(groups.type.target())

    # SIMD builds store 16 one-byte slots; the portable build bit-interleaves
    # the same information into two 64-bit words.
    slot_size = strip_type(group_type["m"].type).target().sizeof
    base = int(groups)
    stride = group_type.sizeof

    for group in range(group_count):
        raw = read_bytes(base + group * stride, GROUP_BYTES)
        last = group == group_count - 1
        if slot_size == 1:
            for slot in range(GROUP_SLOTS):
                meta = raw[slot]
                if meta == 0:
                    continue
                if last and slot == GROUP_SLOTS - 1 and meta == 1:
                    continue  # sentinel
                yield _unwrap_element(elements[group * GROUP_SLOTS + slot])
        else:
            w0 = int.from_bytes(raw[0:8], "little")
            w1 = int.from_bytes(raw[8:16], "little")
            occupied = w0 | w1
            sentinel = (
                last
                and (w0 & _OVERFLOW_MASK) == 0x4000
                and (w1 & _OVERFLOW_MASK) == 0
            )
            for slot in range(GROUP_SLOTS):
                if not occupied & (_OCCUPIED_MASK << slot):
                    continue
                if sentinel and slot == GROUP_SLOTS - 1:
                    continue
                yield _unwrap_element(elements[group * GROUP_SLOTS + slot])


def fca_core(val):
    table = val["table_"]
    found = find_base(table.type, FCA_TABLE)
    return table.cast(found[1]) if found else table


def fca_size(val):
    return int(fca_core(val)["size_"])


def fca_elements(val):
    """Yield the live elements of a closed-addressing table, bucket by bucket."""
    core = fca_core(val)
    size = int(core["size_"])
    array = core["buckets_"]
    bucket_count = int(array["size_"])
    buckets = raw_pointer(array["buckets"])
    if int(buckets) == 0:
        return
    seen = 0
    for i in range(bucket_count):
        node = raw_pointer(buckets[i]["next"])
        while int(node) != 0:
            yield node["buf"]["t_"]
            seen += 1
            if seen > size:
                return  # corrupt chain; stop rather than spin
            node = raw_pointer(node["next"])


class _TablePrinter:
    def __init__(self, val, name, size_fn, elements_fn, is_map):
        self._val = val
        self._name = name
        self._size_fn = size_fn
        self._elements_fn = elements_fn
        self._is_map = is_map

    def to_string(self):
        try:
            size = self._size_fn(self._val)
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._name, exc)
        return "%s with %d element%s" % (self._name, size, "" if size == 1 else "s")

    def children(self):
        return safe_children(self._children())

    def _children(self):
        limit = print_elements_limit()
        for i, element in enumerate(self._elements_fn(self._val)):
            if limit is not None and i >= limit:
                break
            if self._is_map:
                yield ("[%d]" % i, element["first"])
                yield ("[%d]" % i, element["second"])
            else:
                yield ("[%d]" % i, element)

    def display_hint(self):
        return "map" if self._is_map else "array"


class FoaIteratorPrinter:
    """boost::unordered::detail::foa::table_iterator."""

    def __init__(self, val, name):
        self._val = val
        self._name = name

    def to_string(self):
        try:
            ptr = raw_pointer(self._val["p_"])
            if int(ptr) == 0:
                return "%s <end>" % self._name
            return "%s -> %s" % (self._name, _unwrap_element(ptr.dereference()))
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._name, exc)


class FcaIteratorPrinter:
    """boost::unordered::detail::iterator_detail::iterator / c_iterator."""

    def __init__(self, val, name):
        self._val = val
        self._name = name

    def to_string(self):
        try:
            ptr = raw_pointer(self._val["p"])
            if int(ptr) == 0:
                return "%s <end>" % self._name
            return "%s -> %s" % (self._name, ptr["buf"]["t_"])
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._name, exc)


def lookup(val, name):
    if name in FOA_MAPS:
        return _TablePrinter(val, name, foa_size, foa_elements, True)
    if name in FOA_SETS:
        return _TablePrinter(val, name, foa_size, foa_elements, False)
    if name in FCA_MAPS:
        return _TablePrinter(val, name, fca_size, fca_elements, True)
    if name in FCA_SETS:
        return _TablePrinter(val, name, fca_size, fca_elements, False)
    if name == FOA_ITERATOR:
        return FoaIteratorPrinter(val, name)
    if name in FCA_ITERATORS:
        return FcaIteratorPrinter(val, name)
    return None
