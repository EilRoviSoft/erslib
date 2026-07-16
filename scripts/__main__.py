import argparse
import os
import sys
import traceback

import codegen


def add_codegen_subparser(subparsers):
    # Codegen

    codegen_parser = subparsers.add_parser('codegen', help = "Generates dbio code from '*.g.json' descriptors")

    codegen_parser.add_argument("--cwd", default = os.getcwd(), type = str, dest = "cwd")

    # Input: directory that is recursively scanned for '*.g.json' descriptors.
    codegen_parser.add_argument("--dir", type = str, required = True, dest = "dir",
        help = "Directory scanned recursively for '*.g.json' descriptors")

    # Output directories. The descriptor's path relative to '--dir' is preserved
    # under each output directory.
    codegen_parser.add_argument("--hpp-dir", type = str, required = True, dest = "hpp_dir",
        help = "Output directory for generated headers (*.g.hpp)")
    codegen_parser.add_argument("--cpp-dir", type = str, required = True, dest = "cpp_dir",
        help = "Output directory for generated sources (*.g.cpp)")
    codegen_parser.add_argument("--query-dir", type = str, required = True, dest = "query_dir",
        help = "Output directory for generated SQL queries (*.sql)")

    codegen_parser.add_argument("--runtime-namespace", type = str, default = "dbio",
        dest = "runtime_namespace",
        help = "Namespace of the dbio runtime referenced by generated code (default: dbio)")

    codegen_parser.add_argument("--use-query-store", action = "store_true",
        dest = "use_query_store",
        help = "Generate data-access code that looks SQL up from dbio::queries at runtime instead of embedding it as a string literal")


def main():
    parser = argparse.ArgumentParser(prog = "dbio")
    subparsers = parser.add_subparsers(dest = 'command')

    add_codegen_subparser(subparsers)

    # Executing

    args = parser.parse_args()

    output: str

    try:
        match args.command:
            case 'codegen':
                output = codegen.execute(args)

            case _:
                raise ValueError(f"Command {args.command} doesn't exist")
    except Exception as e:
        print(f"Error: {e}\nStacktrace: {traceback.format_exc()}", file = sys.stderr)
        return 1

    print(output, end = "")
    return 0


if __name__ == '__main__':
    sys.exit(main())
