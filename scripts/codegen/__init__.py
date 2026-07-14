import argparse
import json
from pathlib import Path
import os


from .base_codegen import BaseCodegen, GeneratedFile
from .table_codegen import TableCodegen
from .enum_codegen import EnumCodegen
from .query_codegen import QueriesCodegen


variants: dict[str, type[BaseCodegen]] = {
    'table': TableCodegen,
    'enum': EnumCodegen,
    'queries': QueriesCodegen
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

        config.setdefault('runtime_namespace', args.runtime_namespace)

        name = item.name.removesuffix('.g.json')
        relative_dir = item.parent.relative_to(scan_root)

        try:
            if config.get('type') not in variants:
                raise ValueError(f"unknown descriptor type '{config.get('type')}'")

            generator = variants[config['type']](name, config, item.parent)
            generated_files = generator.exec()
        except Exception as error:
            raise RuntimeError(f"{item}: {error}") from error

        for generated in generated_files:
            file_path = _output_path(args, generated, relative_dir, name)

            os.makedirs(os.path.dirname(file_path), exist_ok = True)
            with open(file_path, 'w') as file:
                file.write(generated.content)

            if generated.type == "source":
                generated_sources.append(os.path.abspath(file_path).replace('\\', '/'))

    return ';'.join(generated_sources)
