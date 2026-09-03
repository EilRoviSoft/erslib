#pragma once

// std
#include <array>
#include <charconv>
#include <concepts>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/general.hpp>


// Entry kinds

namespace conduit::impl::reflect::kinds {
    struct table {};
    struct query {};

    struct pk {};
    struct unique {};
    struct fk {};

    struct def {};
    struct identity {};
    struct column {};
}

namespace conduit::impl::reflect {
    template<typename T, typename Kind>
    concept EntryOf = requires { typename T::kind; }
        && std::same_as<typename T::kind, Kind>;


    template<typename T>
    concept TableConstraint = requires { typename T::table_constraint_tag; };

    template<typename T>
    concept ColumnModifier = requires { typename T::column_modifier_tag; };
}


// DDL helpers

namespace conduit::impl::reflect {
    template<auto Value>
    consteval std::string sql_literal() {
        using type = std::remove_cvref_t<decltype(Value)>;

        if constexpr (std::is_same_v<type, bool>) {
            return Value ? "TRUE" : "FALSE";
        }
        else if constexpr (std::is_integral_v<type>) {
            char buffer[32] {};
            const auto [ptr, ec] = std::to_chars(buffer, buffer + sizeof(buffer), Value);
            return std::string(buffer, ptr);
        }
        else {
            static_assert(requires { Value.to_sv(); },
                "Default<> supports bool, integral and fixed_string values only");

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

    consteval std::string field_list(std::span<const std::string_view> what) {
        std::string out = "(";

        for (size_t i = 0; i < what.size(); i++) {
            if (i != 0)
                out += ", ";

            out += what[i];
        }

        out += ')';
        return out;
    }
}


// Declaration tags

namespace conduit::impl::reflect {
    template<ers::fixed_string Name>
    struct Table {
        using kind = kinds::table;
        static constexpr std::string_view name = Name.to_sv();
    };

    template<ers::fixed_string Name>
    struct Query {
        using kind = kinds::query;
        static constexpr std::string_view name = Name.to_sv();
    };
}


// Constraints

namespace conduit::impl::reflect {
    template<ers::fixed_string... Fields>
    struct Pk {
        using kind = kinds::pk;
        using table_constraint_tag = void;

        static constexpr std::array<std::string_view, sizeof...(Fields)> names = { Fields.to_sv()... };

        static consteval std::string ddl(std::string_view name) {
            return "CONSTRAINT " + std::string(name) + " PRIMARY KEY " + field_list(names);
        }
    };

    template<ers::fixed_string... Fields>
    struct Unique {
        using kind = kinds::unique;
        using table_constraint_tag = void;

        static constexpr std::array<std::string_view, sizeof...(Fields)> names = { Fields.to_sv()... };

        static consteval std::string ddl(std::string_view name) {
            return "CONSTRAINT " + std::string(name) + " UNIQUE " + field_list(names);
        }
    };


    enum class EActionOnDelete : u8 {
        none,
        cascade,
        set_null,
        set_default,
        restrict,
    };

    consteval std::string_view on_delete_text(EActionOnDelete what) {
        switch (what) {
            case EActionOnDelete::cascade:     return " ON DELETE CASCADE";
            case EActionOnDelete::set_null:    return " ON DELETE SET NULL";
            case EActionOnDelete::set_default: return " ON DELETE SET DEFAULT";
            case EActionOnDelete::restrict:    return " ON DELETE RESTRICT";
            case EActionOnDelete::none:        return "";
        }

        return "";
    }

    template<
        ers::fixed_string Field,
        ers::fixed_string RefTable,
        ers::fixed_string RefColumn,
        EActionOnDelete OnDelete = EActionOnDelete::none
    >
    struct Fk {
        using kind = kinds::fk;
        using table_constraint_tag = void;

        static constexpr std::string_view field = Field.to_sv();
        static constexpr std::string_view ref_table = RefTable.to_sv();
        static constexpr std::string_view ref_column = RefColumn.to_sv();
        static constexpr EActionOnDelete on_delete = OnDelete;

        static consteval std::string ddl(std::string_view name) {
            return "CONSTRAINT " + std::string(name)
                + " FOREIGN KEY (" + std::string(field)
                + ") REFERENCES " + std::string(ref_table)
                + " (" + std::string(ref_column) + ')'
                + std::string(on_delete_text(on_delete));
        }
    };
}


// Column metadata

namespace conduit::impl::reflect {
    template<ers::fixed_string Field, auto Value>
    struct Default {
        using kind = kinds::def;
        using column_modifier_tag = void;

        static constexpr std::string_view field = Field.to_sv();
        static constexpr auto value = Value;

        static consteval std::string column_ddl() {
            return " DEFAULT " + sql_literal<Value>();
        }
    };

    template<ers::fixed_string Field>
    struct Identity {
        using kind = kinds::identity;
        static constexpr std::string_view field = Field.to_sv();
    };
}


// Annotations utility

namespace conduit::impl::reflect {
    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    struct column_t {
        using kind = kinds::column;
        static constexpr std::string_view name = Name.to_sv();
        static constexpr std::string_view type = Type.to_sv();
    };

    template<ers::fixed_string Name = "", ers::fixed_string Type = "">
    inline constexpr column_t<Name, Type> Column {};


    struct skip_t {};

    inline constexpr skip_t Skip {};
}
