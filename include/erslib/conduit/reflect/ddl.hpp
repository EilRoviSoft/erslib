#pragma once

// std
#include <array>
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
        out += qualified_name<T>;
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
                using entry = entry_type<it>;

                if constexpr (ColumnModifier<entry>)
                    if constexpr (entry_traits<entry>::field == column_name<m>)
                        out += entry_traits<entry>::column_ddl();
            }

            out += ",\n";
        }

        template for (constexpr auto it : declaration_list<T>) {
            using entry = entry_type<it>;

            if constexpr (TableConstraint<entry>) {
                out += "    ";
                out += entry_traits<entry>::ddl({
                    .name = std::define_static_string(std::meta::identifier_of(it)),
                    .schema = schema_name<T>
                });
                out += ",\n";
            }
        }

        if (out.ends_with(",\n"))
            out.resize(out.size() - 2);

        out += "\n)";

        return std::define_static_string(out);
    }


    template<SchemaType S>
    consteval std::string_view create_schema() {
        static_assert(!std::is_same_v<S, NoSchema>, "NoSchema is not a schema that can be created.");

        return std::define_static_string("CREATE SCHEMA IF NOT EXISTS " + std::string(S::name));
    }

    template<SchemaType S>
    consteval auto build_schema_ddl() {
        std::array<std::string_view, entities_of<S>.size() + 1> out {};
        size_t i = 0;

        out[i++] = create_schema<S>();

        template for (constexpr auto it : entities_of<S>)
            out[i++] = create_table<typename [:it:]>();

        return out;
    }

    template<SchemaType S>
    constexpr auto schema_ddl = build_schema_ddl<S>();
}
