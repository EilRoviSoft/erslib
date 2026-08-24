#pragma once

// std
#include <charconv>
#include <span>
#include <string>
#include <string_view>

// ers
#include <erslib/dbio/reflect/schema.hpp>


// Internal

namespace dbio::impl::reflect {
    consteval std::string_view on_delete_text(action_on_delete what) {
        switch (what) {
            case action_on_delete::cascade:     return " ON DELETE CASCADE";
            case action_on_delete::set_null:    return " ON DELETE SET NULL";
            case action_on_delete::set_default: return " ON DELETE SET DEFAULT";
            case action_on_delete::restrict:    return " ON DELETE RESTRICT";
            case action_on_delete::none:        return "";
        }

        return "";
    }

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

    consteval void append_list(std::string& into, std::span<const std::string_view> what) {
        into += '(';

        for (size_t i = 0; i < what.size(); i++) {
            if (i != 0)
                into += ", ";

            into += what[i];
        }

        into += ')';
    }
}


// DDL

namespace dbio::impl::reflect {
    template<Entity T>
    consteval std::string_view create_table() {
        static_assert(
            declaration_is_valid<T>,
            "Declaration names a column that does not exist on the entity"
        );

        std::string out = "CREATE TABLE IF NOT EXISTS ";
        out += table_name<T>;
        out += " (\n";

        template for (constexpr auto m : std::define_static_array(columns<T>())) {
            out += "    ";
            out += column_name<m>;
            out += ' ';
            out += sql_type_name<m>;

            if constexpr (is_identity_column<T, m>)
                out += " GENERATED ALWAYS AS IDENTITY";
            else if constexpr (!is_nullable<m>)
                out += " NOT NULL";

            template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
                using entry = typename [:std::meta::type_of(it):];

                if constexpr (EntryOf<entry, kinds::def>)
                    if constexpr (entry::field == column_name<m>) {
                        out += " DEFAULT ";
                        out += sql_literal<entry::value>();
                    }
            }

            out += ",\n";
        }

        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];
            constexpr std::string_view name = std::define_static_string(std::meta::identifier_of(it));

            if constexpr (EntryOf<entry, kinds::pk>) {
                out += "    CONSTRAINT ";
                out += name;
                out += " PRIMARY KEY ";
                append_list(out, entry::names);
                out += ",\n";
            }
            else if constexpr (EntryOf<entry, kinds::unique>) {
                out += "    CONSTRAINT ";
                out += name;
                out += " UNIQUE ";
                append_list(out, entry::names);
                out += ",\n";
            }
            else if constexpr (EntryOf<entry, kinds::fk>) {
                out += "    CONSTRAINT ";
                out += name;
                out += " FOREIGN KEY (";
                out += entry::field;
                out += ") REFERENCES ";
                out += entry::ref_table;
                out += " (";
                out += entry::ref_column;
                out += ')';
                out += on_delete_text(entry::on_delete);
                out += ",\n";
            }
        }

        if (out.ends_with(",\n"))
            out.resize(out.size() - 2);

        out += "\n)";

        return std::define_static_string(out);
    }
}
