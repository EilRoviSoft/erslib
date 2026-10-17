#pragma once

// std
#include <array>
#include <filesystem>

// ers
#include <erslib/conduit/impl/migrate.hpp>
#include <erslib/conduit/impl/scope.hpp>
#include <erslib/conduit/reflect/ddl.hpp>
#include <erslib/core/hashing/fnv.hpp>


namespace conduit::impl::reflect {
    template<Entity T>
    consteval table_declaration_t table_declaration() {
        constexpr std::string_view text = canonical_declaration<T>();

        return {
            .name = qualified_name<T>,
            .ddl  = create_table<T>(),
            .text = text,
            .hash = ers::Fnv1AHash<std::string_view> {}(text),
        };
    }

    template<SchemaType S>
    consteval auto build_table_declarations() {
        std::array<table_declaration_t, entities_of<S>.size()> out {};
        size_t i = 0;

        template for (constexpr auto it : entities_of<S>) {
            using T = typename [:it:];
            out[i++] = table_declaration<T>();
        }

        return out;
    }

    template<SchemaType S>
    constexpr auto table_declarations = build_table_declarations<S>();


    template<SchemaType S>
    constexpr migration_target_t migration_target {
        .schema        = S::name,
        .create_schema = create_schema<S>(),
        .tables        = table_declarations<S>,
    };


    template<SchemaType... S, ConnectionSource Source>
        requires (sizeof...(S) > 0)
    ers::Result<migration_report_t> migrate(
        Source& source,
        const std::filesystem::path& root,
        const migrate_options_t& options = {}
    ) {
        static constexpr std::array targets { migration_target<S>... };

        return try_with_conn(source, [&](pqxx::connection& conn) {
            return run_migrations(conn, targets, root, options);
        });
    }
}
