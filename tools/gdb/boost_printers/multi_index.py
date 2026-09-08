"""Pretty printers for Boost.MultiIndex.

A ``multi_index_container`` allocates one node per element, and that node is a
stack of per-index link layers::

    ordered_index_node< ... hashed_index_node< sequenced_index_node<
        random_access_index_node< index_node_base<Value, Allocator> > > > >

The outermost layer belongs to index #0, which is also the index the container
itself models, so printing the container means walking the outermost layer.
Every layer carries its links in a ``*_index_node_impl`` base subobject (the
"trampoline"), and link pointers point at that subobject rather than at the
node, so converting back to a node means subtracting the base offset. The
value itself sits in the ``space`` buffer of ``pod_value_holder``, the private
base of ``index_node_base``.

The single header node -- reachable through the ``header_holder`` base of the
container -- serves as the sentinel for every index at once.

Traversal per index kind:

* ordered      -- red-black tree; ``header->left_`` is the first node and the
                  standard in-order successor walk ends back at the header.
                  The node base comes in two flavours: a plain one with
                  ``color_``/``parent_`` and a compressed one that packs the
                  colour into the low bit of ``parentcolor_``.
* sequenced    -- circular doubly linked list through the header.
* random access -- the index owns a pointer array; walk it directly.
* hashed       -- bucket-local chains woven into one traversable sequence;
                  ``after()`` is ported from ``hash_index_node.hpp`` for both
                  the unique and the non-unique node algorithms.
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

CONTAINER = "boost::multi_index::multi_index_container"
HEADER_HOLDER = "boost::multi_index::detail::header_holder"
POD_VALUE_HOLDER = "boost::multi_index::detail::pod_value_holder"
INDEX_BASE = "boost::multi_index::detail::index_base"

ORDERED_INDEX = "boost::multi_index::detail::ordered_index"
HASHED_INDEX = "boost::multi_index::detail::hashed_index"
SEQUENCED_INDEX = "boost::multi_index::detail::sequenced_index"
RANDOM_ACCESS_INDEX = "boost::multi_index::detail::random_access_index"

INDEX_TYPES = (ORDERED_INDEX, HASHED_INDEX, SEQUENCED_INDEX, RANDOM_ACCESS_INDEX)

ORD_IMPL = "boost::multi_index::detail::ordered_index_node_impl"
SEQ_IMPL = "boost::multi_index::detail::sequenced_index_node_impl"
RND_IMPL = "boost::multi_index::detail::random_access_index_node_impl"
HASH_IMPL = "boost::multi_index::detail::hashed_index_node_impl"

IMPL_FOR_INDEX = {
    ORDERED_INDEX: ORD_IMPL,
    HASHED_INDEX: HASH_IMPL,
    SEQUENCED_INDEX: SEQ_IMPL,
    RANDOM_ACCESS_INDEX: RND_IMPL,
}

INDEX_LABEL = {
    ORDERED_INDEX: "ordered",
    HASHED_INDEX: "hashed",
    SEQUENCED_INDEX: "sequenced",
    RANDOM_ACCESS_INDEX: "random access",
}

ITERATORS = (
    "boost::multi_index::detail::bidir_node_iterator",
    "boost::multi_index::detail::hashed_index_iterator",
    "boost::multi_index::detail::rnd_node_iterator",
)


class NodeAccess:
    """Offsets and types needed to move between a node, its link layers and its value."""

    def __init__(self, node_type):
        self.node_type = strip_type(node_type)
        self._impls = {}
        for offset, base in walk_bases(self.node_type):
            name = template_name(base)
            if name in IMPL_FOR_INDEX.values() and name not in self._impls:
                self._impls[name] = (offset, base)

        found = find_base(self.node_type, POD_VALUE_HOLDER)
        if found is None:
            raise GdbError("no %s base in %s" % (POD_VALUE_HOLDER, self.node_type))
        holder_offset, holder = found
        space = None
        for f in holder.fields():
            if f.name == "space" and not f.is_base_class:
                space = f
                break
        if space is None:
            raise GdbError("no space member in %s" % holder)
        self.value_offset = holder_offset + space.bitpos // 8
        self.value_type = strip_type(holder).template_argument(0)
        self._value_ptr = self.value_type.pointer()

    def has_impl(self, impl_name):
        return impl_name in self._impls

    def impl_at(self, node_address, impl_name):
        offset, base = self._impls[impl_name]
        return gdb.Value(node_address + offset).cast(base.pointer())

    def node_address(self, impl_pointer, impl_name):
        return int(impl_pointer) - self._impls[impl_name][0]

    def value_at(self, node_address):
        return gdb.Value(node_address + self.value_offset).cast(self._value_ptr).dereference()

    def value_of_impl(self, impl_pointer, impl_name):
        return self.value_at(self.node_address(impl_pointer, impl_name))


def header_node(container):
    """Pointer to the container's header node (the sentinel shared by all indices)."""
    found = find_base(container.type, HEADER_HOLDER)
    if found is None:
        raise GdbError("no %s base in %s" % (HEADER_HOLDER, strip_type(container.type)))
    return raw_pointer(container.cast(found[1])["member"])


def index_zero_type(container_type):
    """The index #0 class, which is a direct base of the container."""
    for f in strip_type(container_type).fields():
        if f.is_base_class and template_name(f.type) in INDEX_TYPES:
            return strip_type(f.type)
    raise GdbError("no index base in %s" % strip_type(container_type))


def container_of_view(view):
    """Recover the enclosing container from an index view subobject.

    ``index_base<Value, IndexSpecifierList, Allocator>`` carries exactly the
    three arguments the container is spelled with, so the container type can be
    reconstructed and the view's offset inside it looked up.
    """
    found = find_base(view.type, INDEX_BASE)
    if found is None:
        raise GdbError("no %s base in %s" % (INDEX_BASE, strip_type(view.type)))
    base = strip_type(found[1])
    args = [base.template_argument(i) for i in range(3)]
    name = "%s<%s>" % (CONTAINER, ", ".join(str(a) for a in args))
    try:
        container_type = gdb.lookup_type(name)
    except gdb.error:
        raise GdbError("cannot resolve enclosing %s" % CONTAINER)

    address = view.address
    if address is None:
        raise GdbError("index view has no address")

    view_type = strip_type(view.type)
    for offset, candidate in walk_bases(container_type):
        if str(candidate) == str(view_type):
            return (
                gdb.Value(int(address) - offset)
                .cast(container_type.pointer())
                .dereference()
            )
    raise GdbError("index %s is not a base of %s" % (view_type, container_type))


# --------------------------------------------------------------------------
# Per-index traversal. Each yields impl pointers for the live nodes, in the
# order that index's iterators visit them.
# --------------------------------------------------------------------------


def _ordered_nodes(header):
    impl_type = header.type
    compressed = "parentcolor_" in all_field_names(strip_type(impl_type.target()))

    if compressed:
        def parent(node):
            return gdb.Value(int(node["parentcolor_"]) & ~1).cast(impl_type)
    else:
        def parent(node):
            return node["parent_"]

    end = int(header)
    node = header["left_"]
    while int(node) not in (0, end):
        yield node
        right = node["right_"]
        if int(right) != 0:
            node = right
            while int(node["left_"]) != 0:
                node = node["left_"]
        else:
            up = parent(node)
            while int(node) == int(up["right_"]):
                node = up
                up = parent(up)
            if int(node["right_"]) != int(up):
                node = up


def _sequenced_nodes(header):
    end = int(header)
    node = header["next_"]
    while int(node) not in (0, end):
        yield node
        node = node["next_"]


def _random_access_nodes(view):
    pointers = view["ptrs"]
    size = int(pointers["size_"])
    data = raw_pointer(pointers["spc"]["data_"])
    for i in range(size):
        yield data[i]


def _hashed_after(node, impl_type, unique):
    """Port of hashed_index_node_alg<...>::after from hash_index_node.hpp.

    ``next()`` yields a half-header (``prior()`` only) so it is cast back to a
    full node pointer, which is safe because the full node derives from the
    half-header at offset zero.
    """
    def as_node(base_pointer):
        return base_pointer.cast(impl_type)

    nxt = node["next_"]
    prior = nxt.dereference()["prior_"]
    if unique:
        # is_last_of_bucket(x) == (x->next()->prior() != x)
        if int(prior) != int(node):
            return prior
        return as_node(nxt)

    if int(prior) == int(node):
        return as_node(nxt)
    if int(prior["prior_"]) == int(node):
        return prior
    if int(prior["prior_"]["next_"]) == int(node):
        return as_node(nxt)
    return as_node(nxt)["next_"].dereference()["prior_"]


def _hashed_nodes(header, unique):
    impl_type = header.type
    end = int(header)
    node = header["next_"].dereference()["prior_"]
    while int(node) not in (0, end):
        yield node
        node = _hashed_after(node, impl_type, unique)


def _hashed_is_unique(view_type):
    name = str(strip_type(view_type))
    return "hashed_non_unique_tag" not in name


def index_values(view, container, access, kind, limit=None):
    """Yield the values of ``container`` in the order ``view``'s index sees them."""
    impl_name = IMPL_FOR_INDEX[kind]
    if not access.has_impl(impl_name):
        raise GdbError("node %s has no %s layer" % (access.node_type, impl_name))

    if kind == RANDOM_ACCESS_INDEX:
        nodes = _random_access_nodes(view)
    else:
        header = access.impl_at(int(header_node(container)), impl_name)
        if kind == ORDERED_INDEX:
            nodes = _ordered_nodes(header)
        elif kind == SEQUENCED_INDEX:
            nodes = _sequenced_nodes(header)
        else:
            nodes = _hashed_nodes(header, _hashed_is_unique(view.type))

    count = 0
    for node in nodes:
        if limit is not None and count >= limit:
            return
        yield access.value_of_impl(node, impl_name)
        count += 1


class _IndexPrinter:
    """Shared body for the container printer and the index-view printer."""

    def __init__(self, val, label):
        self._val = val
        self._label = label

    def _resolve(self):
        raise NotImplementedError

    def to_string(self):
        try:
            view, container, _, kind = self._resolve()
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._label, exc)
        try:
            size = int(container["node_count"])
            size_text = "%d element%s" % (size, "" if size == 1 else "s")
        except gdb.error:
            size_text = "elements"
        return self._describe(size_text, kind)

    def _describe(self, size_text, kind):
        raise NotImplementedError

    def children(self):
        return safe_children(self._children())

    def _children(self):
        view, container, access, kind = self._resolve()
        # node_count bounds the walk so a corrupt structure cannot spin forever.
        try:
            bound = int(container["node_count"])
        except gdb.error:
            bound = None
        limit = print_elements_limit()
        if limit is not None:
            bound = limit if bound is None else min(bound, limit)
        for i, value in enumerate(index_values(view, container, access, kind, bound)):
            yield ("[%d]" % i, value)

    def display_hint(self):
        return "array"


class ContainerPrinter(_IndexPrinter):
    """boost::multi_index::multi_index_container, shown in index #0 order."""

    def __init__(self, val):
        super().__init__(val, "boost::multi_index_container")

    def _describe(self, size_text, kind):
        return "%s with %s (index #0, %s)" % (self._label, size_text, INDEX_LABEL[kind])

    def _resolve(self):
        container = self._val
        index_type = index_zero_type(container.type)
        access = NodeAccess(header_node(container).type.target())
        return container.cast(index_type), container, access, template_name(index_type)


class ViewPrinter(_IndexPrinter):
    """One index of a multi_index_container, as returned by ``get<Tag>()``."""

    def __init__(self, val, name):
        super().__init__(val, "multi_index %s index" % INDEX_LABEL[name])
        self._kind = name

    def _describe(self, size_text, _kind):
        return "%s with %s" % (self._label, size_text)

    def _resolve(self):
        view = self._val
        container = container_of_view(view)
        access = NodeAccess(header_node(container).type.target())
        return view, container, access, self._kind


class IteratorPrinter:
    """Iterators of every multi_index index share a single ``node`` member."""

    def __init__(self, val, name):
        self._val = val
        self._name = name

    def to_string(self):
        try:
            node = raw_pointer(self._val["node"])
            if int(node) == 0:
                return "%s <null>" % self._name
            access = NodeAccess(node.type.target())
            return "%s -> %s" % (self._name, access.value_at(int(node)))
        except (gdb.error, GdbError) as exc:
            return "%s <%s>" % (self._name, exc)


def lookup(val, name):
    if name == CONTAINER:
        return ContainerPrinter(val)
    if name in INDEX_TYPES:
        return ViewPrinter(val, name)
    if name in ITERATORS:
        return IteratorPrinter(val, name)
    return None
