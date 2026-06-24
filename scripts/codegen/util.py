import os

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
