from .base_codegen import BaseCodegen, GeneratedFile
from .util import load_templates, to_camel_case
from .query import parse_queries, Query


class QueriesCodegen(BaseCodegen):
    def __init__(self, name: str, data: dict, source_dir):
        self.name = name
        self.namespace = data['namespace']
        self.runtime_namespace: str = data.get('runtime_namespace', 'dbio')
        self.use_query_store: bool = data.get('use_query_store', False)

        self.queries: list[Query] = parse_queries(data, source_dir)

        self.include_groups = QueriesCodegen._default_include_groups()
        self._resolve_includes()

    def _resolve_includes(self):
        includes: set[tuple[str, str, str]] = set(QueriesCodegen._default_includes())

        for query in self.queries:
            for field in query.params:
                QueriesCodegen._scan_type_includes(includes, field, is_member = False)
            for field in (query.result or list()):
                QueriesCodegen._scan_type_includes(includes, field, is_member = True)

        grouped: dict[str, dict[str, list[str]]] = dict()
        for target, group, file in includes:
            grouped.setdefault(target, dict()).setdefault(group, list()).append(file)

        for groups in grouped.values():
            for group, files in groups.items():
                groups[group] = sorted(set(files))

        self.includes = grouped

    @staticmethod
    def _scan_type_includes(includes: set, field, is_member: bool):
        match field.type:
            case 'int16_t' | 'int32_t' | 'int64_t' | 'uint16_t' | 'uint32_t' | 'uint64_t':
                includes.add(('header', 'std', 'cstdint'))
            case 'std::string':
                if is_member:
                    includes.add(('header', 'std', 'string'))

        if is_member and 'nullable' in field.flags:
            includes.add(('header', 'std', 'optional'))

    @staticmethod
    def _default_include_groups():
        return [
            ("std", "library"),
            ("pqxx", "library"),
            ("ers", "library"),
            ("dbio", "library"),
            ("export", "library")
        ]

    @staticmethod
    def _default_includes():
        return [
            ('header', 'std', "string_view"),
            ('header', 'pqxx', "pqxx/pqxx"),
            ('header', 'ers', "erslib/core/type/result.hpp"),
            ('header', 'dbio', "erslib/dbio.hpp"),
            ('header', 'export', "erslib/export.hpp")
        ]

    def exec(self) -> list[GeneratedFile]:
        result: list[GeneratedFile] = list()

        for query in self.queries:
            result.append(GeneratedFile(
                filename = query.name + ".g.sql",
                type = "sql",
                content = query.sql + '\n'
            ))

        result.extend(self._generate_code())
        return result

    def _generate_code(self) -> list[GeneratedFile]:
        env, templates = load_templates("query/code")
        env.filters['to_camel_case'] = to_camel_case

        ctx = {
            "name": self.name,
            "namespace": self.namespace,
            "rt": self.runtime_namespace,
            "queries": self.queries,
            "include_groups": self.include_groups,
            "use_query_store": self.use_query_store,
            "fn": {
                "len": len,
                "to_camel_case": to_camel_case
            }
        }

        header = GeneratedFile(
            filename = self.name + ".g.hpp",
            type = "header",
            content = templates["header"].render(
                includes = self.includes['header'],
                **ctx
            )
        )
        source = GeneratedFile(
            filename = self.name + ".g.cpp",
            type = "source",
            content = templates["source"].render(
                includes = self.includes.get('source', {}),
                linked_header = self.name,
                **ctx
            )
        )

        return [header, source]
