"""Shared helpers for the Boost GDB pretty printers."""

import gdb

__all__ = [
    "GdbError",
    "strip_type",
    "template_name",
    "walk_bases",
    "find_base",
    "raw_pointer",
    "field_names",
    "all_field_names",
    "read_bytes",
    "safe_children",
    "print_elements_limit",
]


class GdbError(RuntimeError):
    """Raised when a value does not have the layout the printer expects."""


def strip_type(t):
    """Resolve typedefs/references/cv-qualifiers down to the underlying type."""
    t = t.strip_typedefs()
    while t.code in (gdb.TYPE_CODE_REF, gdb.TYPE_CODE_RVALUE_REF):
        t = t.target().strip_typedefs()
    return t.unqualified()


def template_name(t):
    """``boost::container::vector<int, void>`` -> ``boost::container::vector``.

    Returns ``None`` for anonymous types.
    """
    if isinstance(t, gdb.Value):
        t = t.type
    t = strip_type(t)
    name = t.tag or t.name
    if name is None:
        return None
    head, _, _ = name.partition("<")
    return head.strip()


def walk_bases(t, offset=0, depth=0):
    """Yield ``(byte_offset, type)`` for ``t`` and every base subobject of it.

    Virtual bases (whose offset is not a compile-time constant) are skipped;
    none of the Boost containers handled here use virtual inheritance.
    """
    t = strip_type(t)
    yield offset, t
    if depth > 32:
        return
    try:
        fields = t.fields()
    except (gdb.error, TypeError):
        return
    for f in fields:
        if not getattr(f, "is_base_class", False):
            continue
        if f.bitpos is None:
            continue
        yield from walk_bases(f.type, offset + f.bitpos // 8, depth + 1)


def find_base(t, name):
    """Find a base subobject by template name.

    Returns ``(byte_offset, type)`` or ``None``. ``t`` itself matches at
    offset 0, which is what makes the flat_set/flat_map asymmetry (base vs.
    member) transparent to callers.
    """
    for offset, base in walk_bases(t):
        if template_name(base) == name:
            return offset, base
    return None


def field_names(t):
    t = strip_type(t)
    try:
        return {f.name for f in t.fields() if f.name and not f.is_base_class}
    except (gdb.error, TypeError):
        return set()


def all_field_names(t):
    """Every data member name reachable on ``t``, including through bases."""
    names = set()
    for _, base in walk_bases(t):
        names |= field_names(base)
    return names


def raw_pointer(val, depth=0):
    """Reduce a (possibly fancy) pointer to a plain GDB pointer value.

    Boost containers are allocator-generic, so ``m_start``/``elements_``/node
    links may be class-type "fancy pointers". Those wrap exactly one raw
    pointer, so unwrapping the single pointer-typed member is enough for the
    stock allocators and for the common fancy-pointer idioms (offset_ptr is
    the notable exception and is not supported).
    """
    if val is None:
        raise GdbError("null pointer value")
    t = strip_type(val.type)
    if t.code == gdb.TYPE_CODE_PTR:
        return val.cast(t)
    if depth > 4:
        raise GdbError("cannot unwrap fancy pointer %s" % t)
    for _, base in walk_bases(t):
        try:
            fields = base.fields()
        except (gdb.error, TypeError):
            continue
        for f in fields:
            if f.is_base_class or not f.name:
                continue
            if strip_type(f.type).code == gdb.TYPE_CODE_PTR:
                return val[f.name]
    for _, base in walk_bases(t):
        for f in getattr(base, "fields", lambda: [])():
            if f.is_base_class or not f.name:
                continue
            try:
                return raw_pointer(val[f.name], depth + 1)
            except GdbError:
                continue
    raise GdbError("cannot unwrap fancy pointer %s" % t)


def read_bytes(address, length):
    return bytes(gdb.selected_inferior().read_memory(address, length))


def print_elements_limit():
    """GDB's ``print elements`` setting, or ``None`` when unlimited."""
    try:
        limit = gdb.parameter("print elements")
    except (gdb.error, ValueError):
        return None
    return None if limit in (0, None) else int(limit)


def safe_children(generator):
    """Wrap a children generator so a bad layout degrades to one error row.

    Without this a half-constructed container turns every ``p`` and every
    locals window refresh into a Python traceback.
    """
    try:
        for item in generator:
            yield item
    except (gdb.error, GdbError, OverflowError, ValueError) as exc:
        yield ("<error>", str(exc))
