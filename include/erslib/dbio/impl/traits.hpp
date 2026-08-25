#pragma once

// std
#include <concepts>

// pqxx
#include <pqxx/params>

// ers
#include <erslib/dbio/impl/binder.hpp>



namespace dbio::impl {
    template<typename T>
    struct sql_type;
}


namespace dbio::impl {
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
    binder_t make_binder(T&& value) {
        return [stored = owned_t<T>(std::forward<T>(value))](pqxx::params& out) {
            sql_value<owned_t<T>>::bind(out, stored);
        };
    }
}


namespace dbio::impl {
    template<typename T>
    struct sql_collector;

    template<typename T>
    concept RowContainer = requires(const pqxx::result& r) {
        typename sql_collector<T>::row_t;
        { sql_collector<T>::collect(r) } -> std::same_as<T>;
    };
}
