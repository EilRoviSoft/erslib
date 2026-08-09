import os
from pathlib import Path

import jinja2


def load_templates(dir: str):
    dir_path = f"{os.path.dirname(__file__)}/{dir}"
    env = jinja2.Environment(
        loader = jinja2.FileSystemLoader(dir_path),
        trim_blocks = True,
        lstrip_blocks = True,
        keep_trailing_newline = True
    )
    templates: dict[str, jinja2.Template] = dict()

    with os.scandir(dir_path) as entries:
        for entry in entries:
            if entry.is_file() and entry.name.endswith('.jinja'):
                try:
                    templates[entry.name.removesuffix('.jinja')] = env.get_template(entry.name)
                except jinja2.TemplateNotFound:
                    print(f"Can't find file {os.path.join(dir_path, entry.name)}")
                    continue
    
    return env, templates


def to_camel_case(s: str):
    return ''.join(word.capitalize() for word in s.split('_'))


def resolve_sql(data: dict, source_dir: Path, owner: str, generated_filename: str) -> tuple[str, str]:
    sql = data.get('sql')
    sql_file = data.get('sql_file')
    if (sql is None) == (sql_file is None):
        raise ValueError(f"{owner}: exactly one of 'sql' or 'sql_file' is required")

    if sql is not None:
        content = sql if isinstance(sql, str) else '\n'.join(sql)
        return content + '\n', generated_filename

    path = source_dir / sql_file
    return path.read_text(), path.name
