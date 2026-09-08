# GDB loader for the Boost pretty printers.
# (gdb) source /path/to/tools/gdb/boost-gdb-printers.py

import os
import sys

_HERE = os.path.dirname(os.path.abspath(__file__))
if _HERE not in sys.path:
    sys.path.insert(0, _HERE)

import boost_printers  # noqa: E402

boost_printers.register()
