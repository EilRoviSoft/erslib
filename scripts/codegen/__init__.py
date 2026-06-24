import argparse
import json
from pathlib import Path
import os
import sys


from .base_codegen import BaseCodegen, GeneratedFile
from .table_codegen import TableCodegen
from .enum_codegen import EnumCodegen


variants: dict[str, type[BaseCodegen]] = {
    'table': TableCodegen,
    'enum': EnumCodegen
}


def _output_path(args: argparse.Namespace, file: GeneratedFile, relative_dir: Path, name: str) -> str:
    match file.type:
        case 'header':
            return os.path.join(args.hpp_dir, relative_dir, file.filename)

        case 'source':
            return os.path.join(args.cpp_dir, relative_dir, file.filename)

        case 'sql':
            return os.path.join(args.query_dir, name, file.filename)

        case _:
            raise ValueError(f"Handling of type '{file.type}' doesn't exist")


def execute(args: argparse.Namespace) -> str:
    scan_root = Path(args.dir)

    generated_sources: list[str] = []

    for item in scan_root.rglob("*"):
        if not item.is_file() or not item.name.endswith('.g.json'):
            continue

        config: dict
        with open(item, 'r') as file:
            config = json.load(file)

        if config['type'] not in variants:
            print(f"Not implemented config for {config['type']}", file = sys.stderr)
            continue

        # Let the descriptor opt into a different runtime namespace; otherwise use
        # the one provided on the command line.
        config.setdefault('runtime_namespace', args.runtime_namespace)

        name = item.name.removesuffix('.g.json')

        # Path of the descriptor relative to the scan root, so the same folder
        # structure is mirrored under the header/source output directories.
        relative_dir = item.parent.relative_to(scan_root)

        generator = variants[config['type']](name, config)

        for generated in generator.exec():
            file_path = _output_path(args, generated, relative_dir, name)

            os.makedirs(os.path.dirname(file_path), exist_ok = True)
            with open(file_path, 'w') as file:
                file.write(generated.content)

            if generated.type == "source":
                generated_sources.append(os.path.abspath(file_path).replace('\\', '/'))

    return ';'.join(generated_sources)
