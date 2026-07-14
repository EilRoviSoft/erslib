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


def resolve_sql(data: dict, source_dir: Path, owner: str) -> str:
    has_inline = 'sql' in data
    has_file = 'sql_file' in data

    if has_inline == has_file:
        raise ValueError(f"{owner}: exactly one of 'sql' or 'sql_file' must be provided")

    if has_inline:
        sql = data['sql']
        if not isinstance(sql, str):
            raise ValueError(f"{owner}: 'sql' must be a string or a list of strings")
    else:
        path = source_dir / data['sql_file']
        if not path.is_file():
            raise ValueError(f"{owner}: sql_file not found: {path}")
        sql = path.read_text()

    sql = sql.strip()
    if not sql:
        raise ValueError(f"{owner}: SQL is empty")

    return sql
