from .base_codegen import BaseCodegen, GeneratedFile
from .util import load_templates, to_camel_case


class EnumCodegen(BaseCodegen):
    def __init__(self, name: str, data: dict):
        self.name = name
        self.namespace = data['namespace']

        self.values = data['values']

        self.inherit_type: str = data.get('inherit_type', 'uint32_t')
        self.default: str | None = data.get('default', None)
        self.generate_sql = data.get('generate_sql', False)

        self.runtime_namespace: str = data.get('runtime_namespace', 'dbio')

    def exec(self) -> list[GeneratedFile]:
        result: list[GeneratedFile] = []

        if self.generate_sql:
            result.extend(self._generate_query())
        result.extend(self._generate_header())

        return result

    def _default_context(self):
        return {
            "name": self.name,
            "namespace": self.namespace,
            "typename": to_camel_case(self.name),
            "inherit_type": self.inherit_type,
            "default": self.default,
            "rt": self.runtime_namespace,
            "enum": self,
            "fn": {
                "len": len,
                "to_camel_case": to_camel_case
            }
        }

    def _generate_query(self) -> list[GeneratedFile]:
        env, templates = load_templates("enum/sql")
        env.filters['to_camel_case'] = to_camel_case

        return [GeneratedFile(
            filename = self.name + ".sql",
            type = "sql",
            content = templates["create_enum"].render(**self._default_context())
        )]

    def _generate_header(self) -> list[GeneratedFile]:
        env, templates = load_templates("enum/code")
        env.filters['to_camel_case'] = to_camel_case

        return [GeneratedFile(
            filename = self.name + ".g.hpp",
            type = "header",
            content = templates["header"].render(**self._default_context())
        )]
