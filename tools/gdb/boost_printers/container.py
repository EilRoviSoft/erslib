"""Pretty printers for Boost.Container's vector family and flat containers.

Layout notes (Boost 1.91):

* ``boost::container::vector`` keeps everything in ``m_holder``, a
  ``vector_alloc_holder`` deriving from the allocator. Version-1 allocators
  give it ``m_start`` / ``m_size`` / ``m_capacity``; version-0 allocators --
  which is how ``static_vector`` is built -- drop ``m_start`` and keep the
  elements inside the allocator's own ``storage`` buffer.
* ``small_vector`` derives from ``vector`` via ``small_vector_base`` and uses a
  version-1 allocator, so it needs no special handling.
* ``flat_set``/``flat_multiset`` *derive* from ``dtl::flat_tree`` while
  ``flat_map``/``flat_multimap`` hold one as the ``m_flat_tree`` member. Either
  way the elements are a plain sorted sequence in ``m_data.m_seq``.
"""

import gdb

from ._util import (
    GdbError,
    all_field_names,
    find_base,
    print_elements_limit,
    raw_pointer,
    safe_children,
    strip_type,
    template_name,
    walk_bases,
)

VECTOR = "boost::container::vector"
SMALL_VECTOR = "boost::container::small_vector"
STATIC_VECTOR = "boost::container::static_vector"
FLAT_TREE = "boost::container::dtl::flat_tree"
STATIC_STORAGE_ALLOC = "boost::container::dtl::static_storage_allocator"
VEC_ITERATOR = "boost::container::vec_iterator"

FLAT_SET_TYPES = ("boost::container::flat_set", "boost::container::flat_multiset")
FLAT_MAP_TYPES = ("boost::container::flat_map", "boost::container::flat_multimap")


def vector_storage(val):
    """Return ``(first, size, capacity)`` for any vector-family value.

    ``first`` is a plain pointer to the first element; ``capacity`` may be
    ``None`` when the container does not track one.
    """
    found = find_base(val.type, VECTOR)
    if found is None:
        raise GdbError("not a boost::container::vector: %s" % strip_type(val.type))
    vec = val.cast(found[1])
    holder = vec["m_holder"]
    size = int(holder["m_size"])
    names = all_field_names(holder.type)

    capacity = int(holder["m_capacity"]) if "m_capacity" in names else None

    if "m_start" in names:
        return raw_pointer(holder["m_start"]), size, capacity

    # Version-0 allocator (static_vector): elements live in the allocator base.
    elem_type = strip_type(vec.type).template_argument(0)
    for _, base in walk_bases(holder.type):
        if template_name(base) != STATIC_STORAGE_ALLOC:
            continue
        storage = holder.cast(base)["storage"]
        first = storage["data"].address.cast(elem_type.pointer())
        if capacity is None:
            capacity = int(strip_type(base).template_argument(1))
        return first, size, capacity

    raise GdbError("unrecognised vector_alloc_holder layout")


def vector_elements(val):
    """Yield the elements of any vector-family value, in order."""
    first, size, _ = vector_storage(val)
    for i in range(size):
        yield first[i]


def sequence_elements(val):
    """Yield the elements of a flat container's underlying sequence.

    The sequence defaults to ``boost::container::vector`` but a user may
    substitute any sequence container, so fall back to whatever printer GDB
    already has for it.
    """
    if find_base(val.type, VECTOR) is not None:
        yield from vector_elements(val)
        return

    visualizer = gdb.default_visualizer(val)
    if visualizer is None or not hasattr(visualizer, "children"):
        raise GdbError("no printer for flat container sequence %s" % strip_type(val.type))
    for _, element in visualizer.children():
        yield element


def flat_tree_of(val):
    """Get the ``flat_tree`` subobject, whether inherited or held by value."""
    found = find_base(val.type, FLAT_TREE)
    if found is not None:
        return val.cast(found[1])
    return val["m_flat_tree"]


def flat_elements(val):
    return sequence_elements(flat_tree_of(val)["m_data"]["m_seq"])


class VectorPrinter:
    """boost::container::vector / small_vector / static_vector."""

    def __init__(self, val, name):
        self._val = val
        self._name = name

    def to_string(self):
        try:
            _, size, capacity = vector_storage(self._val)
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._name, exc)
        if capacity is None:
            return "%s of length %d" % (self._name, size)
        return "%s of length %d, capacity %d" % (self._name, size, capacity)

    def children(self):
        return safe_children(self._children())

    def _children(self):
        for i, element in enumerate(vector_elements(self._val)):
            yield ("[%d]" % i, element)

    def display_hint(self):
        return "array"


class FlatSetPrinter:
    """boost::container::flat_set / flat_multiset."""

    def __init__(self, val, name):
        self._val = val
        self._name = name

    def to_string(self):
        try:
            count = sum(1 for _ in flat_elements(self._val))
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._name, exc)
        return "%s with %d element%s" % (self._name, count, "" if count == 1 else "s")

    def children(self):
        return safe_children(self._children())

    def _children(self):
        for i, element in enumerate(flat_elements(self._val)):
            yield ("[%d]" % i, element)

    def display_hint(self):
        return "array"


class FlatMapPrinter:
    """boost::container::flat_map / flat_multimap.

    The tree stores ``std::pair<Key, T>`` (a non-const key) even though the
    container's ``value_type`` is ``std::pair<const Key, T>``; printing the
    stored pair is what we want either way.
    """

    def __init__(self, val, name):
        self._val = val
        self._name = name

    def to_string(self):
        try:
            count = sum(1 for _ in flat_elements(self._val))
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._name, exc)
        return "%s with %d element%s" % (self._name, count, "" if count == 1 else "s")

    def children(self):
        return safe_children(self._children())

    def _children(self):
        limit = print_elements_limit()
        for i, element in enumerate(flat_elements(self._val)):
            if limit is not None and i >= limit:
                break
            yield ("[%d]" % i, element["first"])
            yield ("[%d]" % i, element["second"])

    def display_hint(self):
        return "map"


class VecIteratorPrinter:
    """boost::container::vec_iterator -- the iterator of every flat container."""

    def __init__(self, val, name):
        self._val = val
        self._name = name

    def to_string(self):
        try:
            ptr = raw_pointer(self._val["m_ptr"])
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._name, exc)
        if int(ptr) == 0:
            return "%s <null>" % self._name
        return "%s -> %s" % (self._name, ptr.dereference())


def lookup(val, name):
    if name in (VECTOR, SMALL_VECTOR, STATIC_VECTOR):
        return VectorPrinter(val, name)
    if name in FLAT_SET_TYPES:
        return FlatSetPrinter(val, name)
    if name in FLAT_MAP_TYPES:
        return FlatMapPrinter(val, name)
    if name == VEC_ITERATOR:
        return VecIteratorPrinter(val, name)
    return None
