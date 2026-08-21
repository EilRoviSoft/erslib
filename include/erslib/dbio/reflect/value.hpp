#pragma once

// std
#include <utility>

// pqxx
#include <pqxx/params>

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/dbio/impl/binder.hpp>


// Declaration

namespace dbio::impl::reflect {
    template<typename T>
    struct sql_value;
}


// Implementation

namespace dbio::impl::reflect {
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
    struct sql_value<ers::optional<T>> {
        static void bind(pqxx::params& out, const ers::optional<T>& what) {
            if (what)
                sql_value<T>::bind(out, *what);
            else
                out.append();
        }

        static ers::optional<T> read(const auto& field) {
            if (field.is_null())
                return ers::nullopt;

            return sql_value<T>::read(field);
        }
    };
}


// Utility

namespace dbio::impl::reflect {
    template<typename T>
    binder_t value_binder(T value) {
        return [stored = std::move(value)](pqxx::params& out) {
            sql_value<T>::bind(out, stored);
        };
    }
}
