#pragma once

// std
#include <exception>
#include <format>
#include <string_view>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/core/type/result.hpp>
#include <erslib/flux/impl/scope.hpp>
#include <erslib/flux/reflect/ddl.hpp>


namespace flux::impl::reflect {
    template<SchemaType S, ConnectionSource Source>
    ers::Status create_tables_by_schema(Source& source) {
        std::string_view stage = S::name;

        try {
            with_tx(source, [&stage](pqxx::work& tx) {
                tx.exec(create_schema<S>());

                template for (constexpr auto it : entities_of<S>) {
                    using T = typename [:it:];

                    stage = qualified_name<T>;
                    tx.exec(create_table<T>());
                }

                stage = "commit";
            }, std::format("{}/Create", S::name));
        } catch (const std::exception& e) {
            return ers::make_error("Creating schema '{}' failed at '{}': {}", S::name, stage, e.what());
        }

        return ers::ok;
    }
}
