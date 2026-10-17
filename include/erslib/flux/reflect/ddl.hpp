#pragma once

// std
#include <array>
#include <string>
#include <string_view>

// ers
#include <erslib/flux/reflect/schema.hpp>


namespace flux::impl::reflect {
    template<Entity T, std::meta::info M>
    consteval std::string column_definition() {
        std::string out = std::string(column_name<M>) + ' ' + std::string(sql_type_name<M>);

        if constexpr (is_identity_column<T, M>)
            out += " GENERATED ALWAYS AS IDENTITY";
        else if constexpr (!is_nullable<M>)
            out += " NOT NULL";

        template for (constexpr auto it : declaration_list<T>) {
            using entry = entry_type<it>;

            if constexpr (ColumnModifier<entry>)
                if constexpr (entry_traits<entry>::field == column_name<M>)
                    out += entry_traits<entry>::column_ddl();
        }

        return out;
    }

    template<Entity T>
    consteval std::vector<std::string> table_definitions() {
        static_assert(
            declaration_is_valid<T>,
            "Declaration names a column that does not exist on the entity"
        );

        std::vector<std::string> out;

        template for (constexpr auto m : column_list<T>)
            out.emplace_back(column_definition<T, m>());

        template for (constexpr auto it : declaration_list<T>) {
            using entry = entry_type<it>;

            if constexpr (TableConstraint<entry>) {
                out.emplace_back(entry_traits<entry>::ddl({
                    .name = std::define_static_string(std::meta::identifier_of(it)),
                    .schema = schema_name<T>
                }));
            }
        }

        return out;
    }


    template<Entity T>
    consteval std::string_view create_table() {
        auto lines = table_definitions<T>();

        for (auto& it : lines)
            it.insert(0, "    ");

        return std::define_static_string(
            "CREATE TABLE IF NOT EXISTS " + std::string(qualified_name<T>) + " (\n" + join(lines, ",\n") + "\n)"
        );
    }

    template<Entity T>
    consteval std::string_view canonical_declaration() {
        auto lines = table_definitions<T>();
        std::ranges::sort(lines);

        return std::define_static_string(join(lines, "\n"));
    }
}


namespace flux::impl::reflect {
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
