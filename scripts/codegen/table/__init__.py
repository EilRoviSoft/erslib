# Remarks -------------------------------------------------------------------------------------------------------------


class BaseRemark:
    def flatten(self) -> str:
        raise NotImplementedError()

    def is_field_applicable(self) -> bool:
        raise NotImplementedError()
    

class PrimaryKey(BaseRemark):
    def __init__(self, fields: list[str]):
        super().__init__()

        if len(fields) == 0:
            raise ValueError("You should pass 1 or more fields for primary key")
        self.fields = fields
    
    def flatten(self) -> str:
        if self.is_field_applicable():
            return "PRIMARY KEY"
        else:
            return f"PRIMARY KEY ({", ".join(field for field in self.fields)})"
    
    def is_field_applicable(self) -> bool:
        return len(self.fields) == 1

class ForeignKey(BaseRemark):
    def __init__(self, table: str, field: str, on_delete: str | None = None, on_update: str | None = None):
        super().__init__()
        self.table = table
        self.field = field
        self.on_delete = on_delete
        self.on_update = on_update

    def flatten(self) -> str:
        result = f"REFERENCES {self.table} ({self.field})"
        if self.on_delete:
            result += f" ON DELETE {self.on_delete}"
        if self.on_update:
            result += f" ON UPDATE {self.on_update}"
        return result

    def is_field_applicable(self) -> bool:
        return True

class IdentityRemark(BaseRemark):
    def __init__(self):
        super().__init__()
    
    def flatten(self) -> str:
        return "GENERATED ALWAYS AS IDENTITY"
    
    def is_field_applicable(self) -> bool:
        return True

class UniqueRemark(BaseRemark):
    def __init__(self, fields: list[str], name: str | None = None):
        super().__init__()

        if len(fields) == 0:
            raise ValueError("You should pass 1 or more fields for primary key")
        self.fields = fields
        self.fields_map = set(fields)
        
        self.name = name

    def flatten(self) -> str:
        if self.is_field_applicable():
            result = "UNIQUE"
        else:
            result = f"UNIQUE ({", ".join(field for field in self.fields)})"
        return result
    
    def is_field_applicable(self) -> bool:
        return len(self.fields) == 1

class CheckRemark(BaseRemark):
    def __init__(self, expr: str):
        super().__init__()
        self.expr = expr
    
    def flatten(self) -> str:
        return f"CHECK ({self.expr})"
    
    def is_field_applicable(self) -> bool:
        return False

class StatefulRemark(BaseRemark):
    def __init__(self):
        super().__init__()

    def flatten(self) -> str:
        return "NOT NULL"

    def is_field_applicable(self) -> bool:
        return True

class DefaultRemark(BaseRemark):
    def __init__(self, expr: str):
        super().__init__()
        self.expr = expr

    def flatten(self) -> str:
        return f"DEFAULT {self.expr}"

    def is_field_applicable(self) -> bool:
        return True

def get_precedence(cls):
    order = {
        UniqueRemark: 1,
        StatefulRemark: 2,
        DefaultRemark: 3,
        IdentityRemark: 4,
        ForeignKey: 5,
        PrimaryKey: 6,
        CheckRemark: 7
    }

    return order[cls]


# Field ---------------------------------------------------------------------------------------------------------------


class Field:
    to_cpp_type_table = {
        # fundamental types
        'BOOLEAN': 'bool',

        'INT2': 'int16_t',
        'SMALLINT': 'int16_t',

        'INT4': 'int32_t',
        'INT': 'int32_t',

        'INT8': 'int64_t',
        'BIGINT': 'int64_t',

        'FLOAT4': 'float',
        'REAL': 'float',

        'DOUBLE': 'double',
        'FLOAT8': 'double',
        
        # just variations of strings
        'TEXT': 'std::string',
        'VARCHAR': 'std::string',
        'INET': 'std::string',
        'UUID': 'std::string',
    }

    to_arg_type_table = {
        'std::string': 'std::string_view'
    }

    trivially_copyable_types = {
        'bool',
        'int16_t', 'int32_t', 'int64_t',
        'uint16_t', 'uint32_t', 'uint64_t',
        'float', 'double',
        'std::string_view'
    }

    primitive_types = {
        'bool',
        'int16_t', 'int32_t', 'int64_t',
        'uint16_t', 'uint32_t', 'uint64_t',
        'float', 'double',
        'std::string_view'
    }

    @staticmethod
    def make(data: dict):
        return Field(
            data['name'],
            data['type'],
            data.get('explicit_type', None),
            set(data.get('flags', list())),
            data.get('default', None)
        )

    def __init__(
            self,
            name: str,
            original_type: str,
            explicit_type: str | None,
            flags: set[str],
            default: "str | dict | None"
            ):
        if original_type not in Field.to_cpp_type_table:
            raise ValueError(f"Unknown type: {original_type}")

        self.name = name

        self.original_type = original_type
        self.type = explicit_type if explicit_type else Field.to_cpp_type_table[self.original_type]

        self.flags = flags
        if self.type not in Field.trivially_copyable_types:
            self.flags.add('immovable')

        self._init_remarks()

        code_default, sql_default = Field._split_default(default)

        if sql_default is not None:
            self.add_remark(DefaultRemark(sql_default))

        self.default = 'std::nullopt' if not code_default and 'nullable' in self.flags else code_default

    @staticmethod
    def _split_default(default: "str | dict | None") -> "tuple[str | None, str | None]":
        if isinstance(default, dict):
            return default.get('code'), default.get('sql')
        return default, default
    
    def _init_remarks(self):
        self.remarks: list[BaseRemark] = list()

        if 'nullable' not in self.flags:
            self.remarks.append(StatefulRemark())

        self._remarks_by_class = {type(e) for e in self.remarks}
    
    def has_remark(self, cls):
        return cls in self._remarks_by_class

    def add_remark(self, what: BaseRemark):
        self.remarks.append(what)
        self._remarks_by_class.add(type(what))
    
    def flatten_remarks(self) -> str:
        blacklist = set()
        if self.has_remark(PrimaryKey) and self.has_remark(StatefulRemark):
            blacklist.add(StatefulRemark)
        
        visible = [e for e in self.remarks if type(e) not in blacklist]
        return ' '.join(e.flatten() for e in sorted(visible, key = lambda x: get_precedence(type(x))))

    def class_type(self) -> str:
        type = self.type
        if 'nullable' in self.flags:
            type = f"std::optional<{type}>"
        return type
    
    def arg_type(self) -> str:
        type = self.type
        type = Field.to_arg_type_table.get(type, type)

        if type not in Field.primitive_types or 'immovable' in self.flags:
            type = f"const {type}&"

        return type
    
    def assigment_expression(self) -> str:
        expr = self.name

        if 'immovable' not in self.flags:
            expr = f"std::move({expr})"

        return expr


# Layout --------------------------------------------------------------------------------------------------------------


class Layout:
    def __init__(self, name: str, content: dict[str, list[str] | str], type: str | None = None, persistency: str | None = None):
        self.name = name
        self._content = content
        self.type = type
        self.persistency = persistency
    
    def __getitem__(self, index: str):
        return self._content[index]
    
    def __contains__(self, index):
        return index in self._content
    
    bulk_types = {
        "select_by",
        "select_all"
    }

    def is_bulk(self):
        return self.type in Layout.bulk_types


# Table ---------------------------------------------------------------------------------------------------------------


def make_name_from_fields(fields: list[str]) -> str:
    return "_and_".join([field.removesuffix("_id") for field in fields])


class Table:
    def __init__(self, data: dict):
        self.fields: list[Field] = [Field.make(e) for e in data['fields']]
        self.fields_by_name: dict[str, Field] = {e.name: e for e in self.fields}

        self._init_primary_key(data['keys']['primary'])
        self._init_foreign_keys(data['keys'].get('foreign', dict()))

        self._init_remarks(data.get('remarks', list()))

        self._init_layouts(data.get('layouts', list()))
    
    def _init_primary_key(self, data: list):
        pk = PrimaryKey(data)

        if len(pk.fields) == 1:
            self.fields_by_name[data[0]].add_remark(pk)

        self.primary_key = pk
    
    def _init_foreign_keys(self, data: dict):
        for k, v in data.items():
            fk = ForeignKey(v['table'], v['field'], v.get('on_delete'), v.get('on_update'))
            self.fields_by_name[k].add_remark(fk)
    
    def _init_remarks(self, data: dict):
        self.remarks: list[BaseRemark] = list()
        self.unique: list[UniqueRemark] = list()

        for e in data:
            match e['type']:
                case 'UNIQUE':
                    if 'field' in e:
                        temp = UniqueRemark([e['field']])
                        self.fields_by_name[e['field']].add_remark(temp)
                    elif 'fields' in e:
                        temp = UniqueRemark(list(e['fields']), name = "unique_" + '_'.join(e['fields']))
                    else:
                        raise ValueError("Unique constraint should have 'field' or 'fields' param")
                    self.unique.append(temp)
                case 'IDENTITY':
                    field = self.fields_by_name[e['field']]

                    field.add_remark(IdentityRemark())
                    field.default = f"dbio::undefined_id"
                case 'CHECK':
                    self.remarks.append(CheckRemark(e['expr']))

    def _init_layouts(self, data: dict):
        self.layouts: list[Layout] = list()
        
        # helpers

        self.layouts.append(Layout(name = "none", content = { "get": [] }))
        self.layouts.append(Layout(name = "all", content = { "get": [field.name for field in self.fields] }))
        
        # mandatory

        self.layouts.append(Layout(
            name = "select_all",
            content = {
                "get": [field.name for field in self.fields]
            },
            type = "select_all",
            persistency = "in"
        ))

        self.layouts.append(Layout(
            name = "save",
            content = {
                "put": [field.name for field in self.fields if not field.has_remark(PrimaryKey)],
                "get": self.primary_key.fields
            },
            type = "save",
            persistency = "out"
        ))

        self.layouts.append(Layout(
            name = "update",
            content = {
                "put": [field.name for field in self.fields if not field.has_remark(PrimaryKey)],
                "condition": self.primary_key.fields
            },
            type = "update", persistency = "out"
        ))

        self.layouts.append(Layout(
            name = "load_by_" + make_name_from_fields(self.primary_key.fields),
            content = {
                "put": self.primary_key.fields,
                "get": [field.name for field in self.fields if not field.has_remark(PrimaryKey)]
            },
            type = "load_by",
            persistency = "in"
        ))

        # generated by unique fields

        for unique in self.unique:
            self.layouts.append(Layout(
                name = "load_by_" + make_name_from_fields(unique.fields),
                content = {
                    "put": unique.fields,
                    "get": [field.name for field in self.fields if field.name not in unique.fields_map]
                },
                type = "load_by",
                persistency = "in"
            ))

        # custom layouts

        for layout in data:
            match layout['type']:
                case "select_by":
                    put = layout.get("put", set())

                    self.layouts.append(Layout(
                        name = layout['name'],
                        content = {
                            "put": list(put),
                            "get": [field.name for field in self.fields if field.name not in put],
                            "condition": layout['condition']
                        },
                        type = "select_by",
                        persistency = "in"
                    ))
                
                case _:
                    pass
