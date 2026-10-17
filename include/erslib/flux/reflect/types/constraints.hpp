#pragma once

// std
#include <array>
#include <string>
#include <string_view>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/general.hpp>
#include <erslib/flux/reflect/types/entry.hpp>
#include <erslib/flux/reflect/types/sql.hpp>


namespace flux::impl::reflect {
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
