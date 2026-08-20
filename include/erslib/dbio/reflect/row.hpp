#pragma once

// std
#include <array>
#include <meta>
#include <string_view>

// pqxx
#include <pqxx/result>
#include <pqxx/row>

// ers
#include <erslib/dbio/reflect/schema.hpp>
#include <erslib/dbio/reflect/value.hpp>


namespace dbio::impl {
    template<Entity T>
    using ColumnIndex = std::array<int, column_count<T>()>;


    template<Entity T>
    ColumnIndex<T> map_columns(const pqxx::result& from) {
        ColumnIndex<T> out;
        out.fill(-1);

        size_t slot = 0;

        template for (constexpr auto m : std::define_static_array(columns<T>())) {
            constexpr std::string_view name = column_name<T, m>();

            for (pqxx::row_size_type i = 0; i < from.columns(); i++) {
                if (std::string_view(from.column_name(i)) == name) {
                    out[slot] = static_cast<int>(i);
                    break;
                }
            }

            slot++;
        }

        return out;
    }


    template<Entity T>
    void read_row(T& into, pqxx::row_ref row, const ColumnIndex<T>& index) {
        size_t slot = 0;

        template for (constexpr auto m : std::define_static_array(columns<T>())) {
            if (const int at = index[slot]; at >= 0)
                into.[:m:] = sql_value<member_type<m>>::read(row[static_cast<pqxx::row_size_type>(at)]);

            slot++;
        }
    }

    template<Entity T>
    T from_row(pqxx::row_ref row, const ColumnIndex<T>& index) {
        T out {};
        read_row(out, row, index);
        return out;
    }
}
