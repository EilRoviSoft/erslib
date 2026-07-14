from ..util import resolve_sql, to_camel_case
from ..table import Field


VALID_KINDS = {"rows", "row", "statement"}


def _make_field(entry: dict, owner: str) -> Field:
    return Field(
        entry['name'],
        entry['type'],
        entry.get('explicit_type', None),
        set(entry.get('flags', list())),
        None
    )


class Query:
    def __init__(self, data: dict, source_dir):
        self.name = data['name']
        if not self.name.isidentifier():
            raise ValueError(f"Query name '{self.name}' is not a valid identifier")

        self.kind = data.get('kind')
        if self.kind not in VALID_KINDS:
            raise ValueError(
                f"Query '{self.name}': kind must be one of {sorted(VALID_KINDS)}, got '{self.kind}'"
            )

        self.params: list[Field] = list()
        seen: set[str] = set()
        for entry in data.get('params', list()):
            field = _make_field(entry, self.name)
            if 'nullable' in field.flags:
                raise ValueError(f"Query '{self.name}': nullable parameters are not supported")
            if field.name in seen:
                raise ValueError(f"Query '{self.name}': duplicate parameter '{field.name}'")
            seen.add(field.name)
            self.params.append(field)

        self.result: list[Field] | None = None
        if self.kind in ("rows", "row"):
            if 'result' not in data:
                raise ValueError(f"Query '{self.name}': kind '{self.kind}' requires a 'result' list")

            self.result = list()
            result_seen: set[str] = set()
            for entry in data['result']:
                field = _make_field(entry, self.name)
                if field.name in result_seen:
                    raise ValueError(f"Query '{self.name}': duplicate result column '{field.name}'")
                result_seen.add(field.name)
                self.result.append(field)
        elif 'result' in data:
            raise ValueError(f"Query '{self.name}': '{self.kind}' kind cannot have a 'result'")

        self.struct_name = data.get('result_name', to_camel_case(self.name))

        self.sql = resolve_sql(data, source_dir, f"Query '{self.name}'")

    def is_bulk(self) -> bool:
        return self.kind == "rows"

    def has_result(self) -> bool:
        return self.result is not None


def parse_queries(data: dict, source_dir) -> list[Query]:
    queries = [Query(entry, source_dir) for entry in data.get('queries', list())]

    names: set[str] = set()
    structs: set[str] = set()
    for query in queries:
        if query.name in names:
            raise ValueError(f"Duplicate query name '{query.name}'")
        names.add(query.name)

        if query.has_result():
            if query.struct_name in structs:
                raise ValueError(f"Duplicate result struct '{query.struct_name}'")
            structs.add(query.struct_name)

    return queries
