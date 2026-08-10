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
            return os.path.join(args.out_query_dir, name, file.filename)

        case _:
            raise ValueError(f"Handling of type '{file.type}' doesn't exist")


def _process_descriptor(
        args: argparse.Namespace,
        item: Path,
        generated_sources: list[str],
        all_generated_files: list[str]
        ):
    config: dict
    with open(item, 'r') as file:
        config = json.load(file)

    config.setdefault('runtime_namespace', args.runtime_namespace)

    name = item.name.removesuffix('.g.json')
    relative_dir = item.parent.relative_to(args.dir)

    try:
        descriptor_type = config.get('type')
        if descriptor_type not in variants:
            raise ValueError(f"unknown descriptor type '{descriptor_type}'")

        generator = variants[descriptor_type](name, config, Path(args.in_query_dir))
        generated_files = generator.exec()
    except Exception as error:
        raise RuntimeError(f"{item}: {error}") from error

    for generated in generated_files:
        file_path = _output_path(args, generated, relative_dir, name)

        os.makedirs(os.path.dirname(file_path), exist_ok = True)
        with open(file_path, 'w') as file:
            file.write(generated.content)

        abs_path = os.path.abspath(file_path).replace('\\', '/')

        if generated.type == "source":
            generated_sources.append(abs_path)

        all_generated_files.append(abs_path)


def execute(args: argparse.Namespace) -> str:
    generated_sources: list[str] = []
    all_generated_files: list[str] = []

    for item in Path(args.dir).rglob("*.g.json"):
        if not item.is_file():
            continue

        _process_descriptor(args, item, generated_sources, all_generated_files)

    file_list = getattr(args, 'file_list', None)
    if file_list:
        os.makedirs(os.path.dirname(os.path.abspath(file_list)) or '.', exist_ok = True)
        with open(file_list, 'w') as file:
            file.write('\n'.join(all_generated_files))
            if all_generated_files:
                file.write('\n')

    return ';'.join(generated_sources)
