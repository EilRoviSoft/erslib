#pragma once

// std
#include <span>
#include <string>
#include <string_view>
#include <vector>

// ers
#include <erslib/dbio/reflect/traits.hpp>


// Internal

namespace dbio::impl {
    consteval std::string_view on_delete_text(OnDelete what) {
        switch (what) {
            case OnDelete::Cascade:  return " ON DELETE CASCADE";
            case OnDelete::SetNull:  return " ON DELETE SET NULL";
            case OnDelete::Restrict: return " ON DELETE RESTRICT";
            case OnDelete::NoAction: return "";
        }

        return "";
    }

    consteval void append_list(std::string& into, std::span<const char* const> what) {
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

namespace dbio::impl {
    template<Entity T>
    consteval std::string_view create_table() {
        std::string out = "CREATE TABLE IF NOT EXISTS ";
        out += table_name<T>();
        out += " (\n";

        template for (constexpr auto m : std::define_static_array(columns<T>())) {
            out += "    ";
            out += column_name<m>();
            out += ' ';
            out += sql_type_of<m>();

            if constexpr (is_identity<m, T>())
                out += " GENERATED ALWAYS AS IDENTITY";

            if constexpr (!is_nullable_column<m>() && !is_identity<m, T>())
                out += " NOT NULL";

            if constexpr (has_text<m, Default>()) {
                out += " DEFAULT ";
                out += text_of<m, Default>();
            }

            out += ",\n";
        }

        constexpr auto primary = primary_key<T>();
        if constexpr (!primary.empty()) {
            out += "    PRIMARY KEY ";
            append_list(out, primary);
            out += ",\n";
        }

        for (const auto& group : unique_groups<T>()) {
            out += "    UNIQUE ";
            append_list(out, group);
            out += ",\n";
        }

        template for (constexpr auto m : std::define_static_array(columns<T>())) {
            template for (constexpr auto a : std::define_static_array(std::meta::annotations_of(m))) {
                constexpr auto type = std::meta::remove_cv(std::meta::type_of(a));

                if constexpr (std::meta::has_template_arguments(type) && std::meta::template_of(type) == ^^Fk) {
                    out += "    FOREIGN KEY (";
                    out += column_name<m>();
                    out += ") REFERENCES ";
                    out += table_name<typename [:type:]::target>();
                    out += " (";

                    out += [:type:]::column;

                    out += ')';
                    out += on_delete_text([:type:]::on_delete);
                    out += ",\n";
                }
            }
        }

        if (out.ends_with(",\n"))
            out.resize(out.size() - 2);

        out += "\n)";

        return std::define_static_string(out);
    }
}


// Exports

namespace dbio::ddl {
    using impl::create_table;
}
