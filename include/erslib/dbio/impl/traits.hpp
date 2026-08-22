#pragma once

// pqxx
#include <pqxx/params>

// ers
#include <erslib/dbio/impl/binder.hpp>


namespace dbio::impl {
    template<typename T>
    struct sql_type;

    template<typename T>
    struct sql_value {
        static void bind(pqxx::params& out, const T& what) {
            out.append(what);
        }

        static T read(const auto& field) {
            return field.template as<T>();
        }
    };


    template<typename T>
    binder_t value_binder(T value) {
        return [stored = std::move(value)](pqxx::params& out) {
            sql_value<T>::bind(out, stored);
        };
    }
}
