import os

from .base_codegen import BaseCodegen, GeneratedFile
from .util import load_templates, to_camel_case
from .table import Table, Field, PrimaryKey, IdentityRemark, StatefulRemark


class TableCodegen(BaseCodegen):
    def __init__(self, name: str, data: dict, source_dir):
        self.name = name
        self.namespace = data['namespace']
        self.table_name = data.get('table_name', self.name + 's')
        self.use_query_store: bool = data.get('use_query_store', False)

        self.table = Table(data, source_dir)

        self.include_groups = TableCodegen._default_include_groups()
        self._resolve_includes()

    def _resolve_includes(self):
        includes: set[tuple[str, str, str]] = set(TableCodegen._default_includes())

        for field in self.table.fields:
            if not field.has_remark(StatefulRemark) and not field.has_remark(PrimaryKey):
                includes.add(('header', 'std', 'optional'))

            match field.type:
                case 'int16_t' | 'int32_t' | 'int64_t':
                    includes.add(('header', 'std', 'cstdint'))

                case 'std::string':
                    includes.add(('header', 'std', 'string'))

        for layout in self.table.layouts:
            for param in layout.params:
                match param.type:
                    case 'int16_t' | 'int32_t' | 'int64_t' | 'uint16_t' | 'uint32_t' | 'uint64_t':
                        includes.add(('header', 'std', 'cstdint'))

        grouped: dict[str, dict[str, list[str]]] = dict()
        for target, group, file in includes:
            grouped.setdefault(target, dict()).setdefault(group, list()).append(file)

        for groups in grouped.values():
            for group, files in groups.items():
                groups[group] = sorted(set(files))

        self.includes = grouped

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
            ('header', 'std', "bitset"),
            ('header', 'std', "string_view"),
            ('header', 'pqxx', "pqxx/pqxx"),
            ('header', 'ers', "erslib/core/type/result.hpp"),
            ('header', 'dbio', "erslib/dbio.hpp"),
            ('header', 'export', "erslib/export.hpp")
        ]

    def exec(self) -> list[GeneratedFile]:
        rendered = self._render_queries()
        return self._generate_sql(rendered) + self._generate_code(rendered)

    def _render_queries(self) -> dict[str, str]:
        _, templates = load_templates("table/sql")
        ctx = self._default_context()

        ctx.update({ "name": self.table_name })

        rendered: dict[str, str] = dict()

        rendered["create"] = templates["create_table"].render(
            properties = self._make_table_properties(),
            **ctx
        )

        for layout in self.table.layouts:
            if not layout.type:
                continue

            if layout.raw_sql is not None:
                rendered[layout.name] = layout.raw_sql + '\n'
            else:
                rendered[layout.name] = templates[layout.type].render(layout = layout, **ctx)

        return rendered

    def _generate_sql(self, rendered: dict[str, str]) -> list[GeneratedFile]:
        queries: list[GeneratedFile] = list()

        queries.append(GeneratedFile(
            filename = "create.sql",
            type = "sql",
            content = rendered["create"]
        ))

        for layout in self.table.layouts:
            if layout.type:
                queries.append(GeneratedFile(
                    filename = layout.name + ".sql",
                    type = "sql",
                    content = rendered[layout.name]
                ))

        return queries

    def _make_table_properties(self):
        result = [
            f"{e.name} {e.original_type}" + (' ' + e.flatten_remarks() if len(e.remarks) > 0 else "")
            for e in self.table.fields
        ]

        if not self.table.primary_key.is_field_applicable():
            result.append(self.table.primary_key.flatten())

        result.extend(e.flatten() for e in self.table.remarks)
        result.extend(e.flatten() for e in self.table.unique if len(e.fields) > 1)

        return result

    def _generate_code(self, rendered: dict[str, str]) -> list[GeneratedFile]:
        env, templates = load_templates("table/code")

        env.filters['to_camel_case'] = to_camel_case

        queries = {
            layout.name: rendered[layout.name][:-1]
            for layout in self.table.layouts if layout.type
        }

        ctx = self._default_context()
        ctx.update({
            "name": self.name,
            "namespace": self.namespace,
            "include_groups": self.include_groups,
            "queries": queries,
            "use_query_store": self.use_query_store
        })

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

    def _default_context(self):
        def is_primary_key(field: Field):
            return field.has_remark(PrimaryKey)

        return {
            "table": self.table,
            "fn": {
                "len": len,
                "to_camel_case": to_camel_case,
                "is_primary_key": is_primary_key
            }
        }
