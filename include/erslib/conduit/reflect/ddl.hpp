#pragma once

// std
#include <string>
#include <string_view>

// ers
#include <erslib/conduit/reflect/schema.hpp>


namespace conduit::impl::reflect {
    template<Entity T>
    consteval std::string_view create_table() {
        static_assert(
            declaration_is_valid<T>,
            "Declaration names a column that does not exist on the entity"
        );

        std::string out = "CREATE TABLE IF NOT EXISTS ";
        out += table_name<T>;
        out += " (\n";

        template for (constexpr auto m : column_list<T>) {
            out += "    ";
            out += column_name<m>;
            out += ' ';
            out += sql_type_name<m>;

            if constexpr (is_identity_column<T, m>)
                out += " GENERATED ALWAYS AS IDENTITY";
            else if constexpr (!is_nullable<m>)
                out += " NOT NULL";

            template for (constexpr auto it : declaration_list<T>) {
                using entry = typename [:std::meta::type_of(it):];

                if constexpr (ColumnModifier<entry>)
                    if constexpr (entry::field == column_name<m>)
                        out += entry::column_ddl();
            }

            out += ",\n";
        }

        template for (constexpr auto it : declaration_list<T>) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (TableConstraint<entry>) {
                out += "    ";
                out += entry::ddl(std::define_static_string(std::meta::identifier_of(it)));
                out += ",\n";
            }
        }

        if (out.ends_with(",\n"))
            out.resize(out.size() - 2);

        out += "\n)";

        return std::define_static_string(out);
    }
}
