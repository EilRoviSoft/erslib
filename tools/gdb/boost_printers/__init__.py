"""GDB pretty printers for Boost containers.

Covers Boost.Container (the vector family and the flat associative
containers), Boost.Unordered (open- and closed-addressing) and
Boost.MultiIndex. Verified against Boost 1.91.

The printers are registered *globally* rather than per-objfile. GDB consults
objfile printers first, so this deliberately leaves Boost's own embedded
printers (``.debug_gdb_scripts``, Boost >= 1.87, Boost.Unordered only) in
charge whenever they are present.

Usage::

    (gdb) source /path/to/tools/gdb/boost-gdb-printers.py

or from Python::

    import boost_printers
    boost_printers.register()
"""

import gdb
import gdb.printing

from . import container, multi_index, unordered
from ._util import strip_type, template_name

__all__ = ["BoostPrinter", "register"]

_MODULES = (container, unordered, multi_index)


class BoostPrinter(gdb.printing.PrettyPrinter):
    """Dispatches on a value's template name, ignoring template arguments."""

    def __init__(self, name="boost"):
        super().__init__(name)

    def __call__(self, val):
        try:
            value_type = strip_type(val.type)
        except gdb.error:
            return None
        if value_type.code not in (gdb.TYPE_CODE_STRUCT, gdb.TYPE_CODE_UNION):
            return None

        name = template_name(value_type)
        if name is None or not name.startswith("boost::"):
            return None

        try:
            val = val.cast(value_type)
        except gdb.error:
            return None

        for module in _MODULES:
            printer = module.lookup(val, name)
            if printer is not None:
                return printer
        return None


def register(objfile=None, name="boost"):
    """Register the printers. ``objfile=None`` means globally."""
    gdb.printing.register_pretty_printer(objfile, BoostPrinter(name), replace=True)
