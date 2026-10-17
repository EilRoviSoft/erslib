#pragma once

// std
#include <array>
#include <charconv>
#include <concepts>
#include <ranges>
#include <span>
#include <string>
#include <type_traits>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/general.hpp>


// Entry kinds

namespace conduit::impl::reflect::kinds {
    struct ESchema {};
    struct ETable {};
    struct EQuery {};

    struct EPk {};
    struct EUnique {};
    struct EFk {};
    struct ECheck {};

    struct EDefault {};
    struct EIdentity {};
    struct EColumn {};
}

namespace conduit::impl::reflect {
    template<typename T, typename Kind>
    concept EntryOf = requires { typename T::kind; }
        && std::same_as<typename T::kind, Kind>;
}


// Entry traits

namespace conduit::impl::reflect {
    struct constraint_context {
        std::string_view name;
        std::string_view schema;
    };

    struct entry_defaults {
        static constexpr std::array<std::string_view, 0> columns {};
        static constexpr std::array<std::string_view, 0> references {};
    };

    template<typename E>
    struct entry_traits : entry_defaults {};


    template<typename E>
    concept TableConstraint = requires(const constraint_context& ctx) {
        { entry_traits<E>::ddl(ctx) } -> std::convertible_to<std::string>;
    };

    template<typename E>
    concept ColumnModifier = requires {
        { entry_traits<E>::field } -> std::convertible_to<std::string_view>;
        { entry_traits<E>::column_ddl() } -> std::convertible_to<std::string>;
    };
}


// DDL helpers

namespace conduit::impl::reflect {
    template<ers::fixed_string Sql>
    struct raw_sql_t {
        static constexpr std::string_view sql = Sql.to_sv();
    };

    template<ers::fixed_string Sql>
    inline constexpr raw_sql_t<Sql> raw_sql {};

    template<typename T>
    concept RawSql = requires {
        { T::sql } -> std::convertible_to<std::string_view>;
    };
}

namespace conduit::impl::reflect {
    template<auto Value>
    consteval std::string sql_literal() {
        using type = std::remove_cvref_t<decltype(Value)>;

        if constexpr (RawSql<type>) {
            return std::string(type::sql);
        } else if constexpr (std::is_same_v<type, bool>) {
            return Value ? "TRUE" : "FALSE";
        } else if constexpr (std::is_integral_v<type>) {
            char buffer[32] {};
            const auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), Value);
            return std::string(buffer, ptr);
        } else {
            static_assert(
                requires { Value.to_sv(); },
                "Default<> supports bool, integral, fixed_string and raw_sql values only."
            );

            std::string out = "'";

            for (const char it : Value.to_sv()) {
                if (it == '\'')
                    out += '\'';

                out += it;
            }

            out += '\'';
            return out;
        }
    }

    consteval std::string qualify(std::string_view schema, std::string_view table) {
        if (schema.empty() || table.contains('.'))
            return std::string(table);

        return std::string(schema) + '.' + std::string(table);
    }

    template<std::ranges::input_range R>
    consteval std::string join(const R& what, std::string_view separator) {
        std::string out;
        bool first = true;

        for (const auto& it : what) {
            if (!first)
                out += separator;

            out += it;
            first = false;
        }

        return out;
    }

    consteval std::string field_list(std::span<const std::string_view> what) {
        return '(' + join(what, ", ") + ')';
    }
}


// Declaration tags

namespace conduit::impl::reflect {
    template<ers::fixed_string Name>
    struct Schema {
        using kind = kinds::ESchema;
        static constexpr std::string_view name = Name.to_sv();

        static_assert(!Name.to_sv().empty(), "Schema<> needs a name.");
        static_assert(!Name.to_sv().contains('.'), "Schema<> name must not be qualified.");
    };

    struct NoSchema {
        using kind = kinds::ESchema;
        static constexpr std::string_view name = "";
    };

    template<typename T>
    concept SchemaType = EntryOf<T, kinds::ESchema>;


    template<ers::fixed_string Name, SchemaType S = NoSchema>
    struct Table {
        using kind = kinds::ETable;
        using schema = S;

        static constexpr std::string_view name = Name.to_sv();

        static_assert(std::is_same_v<S, NoSchema> || !Name.to_sv().contains('.'),
            "Table<> name must not be qualified when a schema is given, the schema type qualifies it.");
    };

    template<ers::fixed_string Name>
    struct Query {
        using kind = kinds::EQuery;
        static constexpr std::string_view name = Name.to_sv();
    };
}


// Constraints

namespace conduit::impl::reflect {
    // Pk

    template<ers::fixed_string... Fields>
    struct Pk {
        using kind = kinds::EPk;
    };

    template<ers::fixed_string... Fields>
    struct entry_traits<Pk<Fields...>> : entry_defaults {
        static constexpr std::array<std::string_view, sizeof...(Fields)> columns = { Fields.to_sv()... };

        static consteval std::string ddl(const constraint_context& ctx) {
            return "CONSTRAINT " + std::string(ctx.name) + " PRIMARY KEY " + field_list(columns);
        }
    };


    // Unique

    template<ers::fixed_string... Fields>
    struct Unique {
        using kind = kinds::EUnique;
    };

    template<ers::fixed_string... Fields>
    struct entry_traits<Unique<Fields...>> : entry_defaults {
        static constexpr std::array<std::string_view, sizeof...(Fields)> columns = { Fields.to_sv()... };

        static consteval std::string ddl(const constraint_context& ctx) {
            return "CONSTRAINT " + std::string(ctx.name) + " UNIQUE " + field_list(columns);
        }
    };


    // Fk

    enum class EActionOnDelete : u8 {
        None,
        Cascade,
        SetNull,
        SetDefault,
        Restrict,
    };

    consteval std::string_view on_delete_text(EActionOnDelete what) {
        switch (what) {
            case EActionOnDelete::Cascade:
                return " ON DELETE CASCADE";

            case EActionOnDelete::SetNull:
                return " ON DELETE SET NULL";

            case EActionOnDelete::SetDefault:
                return " ON DELETE SET DEFAULT";

            case EActionOnDelete::Restrict:
                return " ON DELETE RESTRICT";

            default:
                return "";
        }
    }

    template<
        ers::fixed_string Field,
        ers::fixed_string RefTable,
        ers::fixed_string RefColumn,
        EActionOnDelete OnDelete = EActionOnDelete::None
    >
    struct Fk {
        using kind = kinds::EFk;
    };

    template<
        ers::fixed_string Field,
        ers::fixed_string RefTable,
        ers::fixed_string RefColumn,
        EActionOnDelete OnDelete
    >
    struct entry_traits<Fk<Field, RefTable, RefColumn, OnDelete>> : entry_defaults {
        static constexpr std::array<std::string_view, 1> columns = { Field.to_sv() };
        static constexpr std::array<std::string_view, 1> references = { RefTable.to_sv() };

        static consteval std::string ddl(const constraint_context& ctx) {
            return "CONSTRAINT " + std::string(ctx.name)
                + " FOREIGN KEY (" + std::string(Field.to_sv())
                + ") REFERENCES " + qualify(ctx.schema, RefTable.to_sv())
                + " (" + std::string(RefColumn.to_sv()) + ')'
                + std::string(on_delete_text(OnDelete));
        }
    };


    // Check

    template<ers::fixed_string Expr>
    struct Check {
        using kind = kinds::ECheck;
    };

    template<ers::fixed_string Expr>
    struct entry_traits<Check<Expr>> : entry_defaults {
        static consteval std::string ddl(const constraint_context& ctx) {
            return "CONSTRAINT " + std::string(ctx.name)
                + " CHECK (" + std::string(Expr.to_sv()) + ")";
        }
    };
}


// Column metadata

namespace conduit::impl::reflect {
    // Default

    template<ers::fixed_string Field, auto Value>
    struct Default {
        using kind = kinds::EDefault;
    };

    template<ers::fixed_string Field, auto Value>
    struct entry_traits<Default<Field, Value>> : entry_defaults {
        static constexpr std::string_view field = Field.to_sv();
        static constexpr std::array<std::string_view, 1> columns = { field };

        static consteval std::string column_ddl() {
            return " DEFAULT " + sql_literal<Value>();
        }
    };


    // Identity

    template<ers::fixed_string Field>
    struct Identity {
        using kind = kinds::EIdentity;
    };

    template<ers::fixed_string Field>
    struct entry_traits<Identity<Field>> : entry_defaults {
        static constexpr std::string_view field = Field.to_sv();
        static constexpr std::array<std::string_view, 1> columns = { field };
    };
}


// Annotations utility

namespace conduit::impl::reflect {
    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    struct column_t {
        using kind = kinds::EColumn;
        static constexpr std::string_view name = Name.to_sv();
        static constexpr std::string_view type = Type.to_sv();
    };

    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    inline constexpr column_t<Name, Type> Column {};


    struct skip_t {};

    inline constexpr skip_t Skip {};
}
