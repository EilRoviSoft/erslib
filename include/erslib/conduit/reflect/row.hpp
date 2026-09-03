#pragma once

// std
#include <array>
#include <meta>
#include <string_view>
#include <type_traits>

// pqxx
#include <pqxx/result>
#include <pqxx/row>

// ers
#include <erslib/conduit/impl/generator.hpp>
#include <erslib/conduit/impl/traits.hpp>
#include <erslib/conduit/reflect/schema.hpp>


namespace conduit::impl::reflect {
    template<typename T>
    concept PlainRow = std::is_aggregate_v<T> && !ValidRow<T> && !RowType<T>;


    template<typename T>
    concept MappedRow = (RowType<T> || PlainRow<T>) && !ValidRow<T>;


    template<typename T>
    using ColumnIndex = std::array<int, column_count<T>>;


    template<typename T>
    ColumnIndex<T> map_columns(const pqxx::result& from) {
        ColumnIndex<T> out;
        out.fill(-1);

        size_t slot = 0;

        template for (constexpr auto m : column_list<T>) {
            constexpr std::string_view name = column_name<m>;

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


    template<typename T>
    void read_row(T& into, pqxx::row_ref row, const ColumnIndex<T>& index) {
        size_t slot = 0;

        template for (constexpr auto m : column_list<T>) {
            if (const int at = index[slot]; at >= 0)
                into.[:m:] = sql_value<member_type<m>>::read(row[static_cast<pqxx::row_size_type>(at)]);

            slot++;
        }
    }

    template<typename T>
    T from_row(pqxx::row_ref row, const ColumnIndex<T>& index) {
        T out {};
        read_row(out, row, index);
        return out;
    }
}


template<conduit::impl::reflect::MappedRow T>
struct conduit::impl::row_reader<T> {
    using state = reflect::ColumnIndex<T>;

    static state prepare(const pqxx::result& from) { return reflect::map_columns<T>(from); }
    static T read(pqxx::row_ref row, const state& index) { return reflect::from_row<T>(row, index); }
};
