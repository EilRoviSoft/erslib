import os
import sys

import gdb

_TOOLS = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
if _TOOLS not in sys.path:
    sys.path.insert(0, _TOOLS)

import boost_printers

boost_printers.register()

_failures = []
_checks = 0
_skipped = 0


def _visualizer(expr):
    printer = gdb.default_visualizer(gdb.parse_and_eval(expr))
    if printer is None:
        raise AssertionError("no printer registered")
    return printer


def check(expr, count, expected=None, key=None, pairs=False, ordered=True, when=True):
    global _checks, _skipped
    if not when:
        _skipped += 1
        return
    _checks += 1
    key = key or (lambda v: v)
    try:
        printer = _visualizer(expr)
        header = printer.to_string()
        children = [value for _, value in printer.children()]
        if pairs:
            children = children[0::2]
        if len(children) != count:
            raise AssertionError("%d children, expected %d [%s]" % (len(children), count, header))
        if expected is not None:
            got = [str(key(v)) for v in children]
            want = [str(v) for v in expected]
            if not ordered:
                got, want = sorted(got), sorted(want)
            if got != want:
                raise AssertionError("contents differ\n    got  %s\n    want %s" % (got, want))
        print("ok    %-14s %s" % (expr, header))
    except Exception as exc:
        _failures.append(expr)
        print("FAIL  %-14s %s" % (expr, exc))


def to_string(expr, when=True):
    global _checks, _skipped
    if not when:
        _skipped += 1
        return
    _checks += 1
    try:
        print("ok    %-14s %s" % (expr, _visualizer(expr).to_string()))
    except Exception as exc:
        _failures.append(expr)
        print("FAIL  %-14s %s" % (expr, exc))


def is_ours(expr):
    try:
        printer = gdb.default_visualizer(gdb.parse_and_eval(expr))
    except gdb.error:
        return False
    return printer is not None and type(printer).__module__.startswith("boost_printers")


rec_id = lambda v: v["id"]
point_x = lambda v: v["x"]
UNORDERED = is_ours("ufs")

print("-- Boost.MultiIndex")
check("full", 7, range(7), rec_id)
check("by_id_idx", 7, range(7), rec_id)
check("by_name_idx", 7, range(7), rec_id, ordered=False)
check("by_seq_idx", 7, range(7), rec_id)
check("by_rnd_idx", 7, range(7), rec_id)
check("ho", 40, range(40), rec_id, ordered=False)
check("hmo", 40, range(40), rec_id, ordered=False)
check("so", 4, range(4), rec_id)
check("ro", 4, range(4), rec_id)
check("om", 6, range(6), rec_id, ordered=False)
check("empty_full", 0, [])
check("empty_ho", 0, [])
check("empty_so", 0, [])
check("empty_ro", 0, [])
to_string("mi_it")

if UNORDERED:
    print("-- Boost.Unordered, open addressing")
else:
    print("-- Boost.Unordered: SKIPPED, the binary's embedded Boost printers are in")
    print("   charge. Rebuild with -DBOOST_ALL_NO_EMBEDDED_GDB_SCRIPTS to exercise these.")

check("ufm", 40, range(40), pairs=True, ordered=False, when=UNORDERED)
check("ufs", 40, range(40), ordered=False, when=UNORDERED)
check("unm", 20, range(20), pairs=True, ordered=False, when=UNORDERED)
check("uns", 2, ['"a"', '"b"'], ordered=False, when=UNORDERED)
check("ufs_custom", 3, [1, 2, 3], point_x, ordered=False, when=UNORDERED)
check("ufm_empty", 0, [], when=UNORDERED)
to_string("foa_it", when=UNORDERED)

if UNORDERED:
    print("-- Boost.Unordered, closed addressing")
check("um", 30, range(30), pairs=True, ordered=False, when=UNORDERED)
check("umm", 10, [0, 0, 0, 0, 1, 1, 1, 2, 2, 2], pairs=True, ordered=False, when=UNORDERED)
check("us", 30, range(30), ordered=False, when=UNORDERED)
check("um_empty", 0, [], when=UNORDERED)
to_string("fca_it", when=UNORDERED)

print("-- Boost.Container")
check("fm", 3, [1, 2, 3], pairs=True)
check("fmm", 2, [1, 1], pairs=True)
check("fm_empty", 0, [])
check("fs_small", 6, [1, 2, 3, 4, 5, 6])
check("fs_static", 3, [1, 2, 3])
check("fs_std", 3, [3, 4, 5])
check("fm_std", 2, [1, 2], pairs=True)
check("fs_custom", 3, [1, 2, 3], point_x)
check("bvs", 2, ['"alpha"', '"beta"'])
check("bv_empty", 0, [])
check("sv_inline", 2, [1, 2])
check("sv_heap", 5, [1, 2, 3, 4, 5])
check("stv", 3, [9, 8, 7])
check("stv_empty", 0, [])
to_string("fm_it")

print("\n%d checks, %d skipped, %d failure%s"
    % (_checks, _skipped, len(_failures), "" if len(_failures) == 1 else "s"))

if _failures:
    print("failed: %s" % ", ".join(_failures))
    gdb.execute("quit 1")
