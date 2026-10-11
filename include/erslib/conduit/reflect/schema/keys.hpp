#pragma once

// std
#include <meta>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

// ers
#include <erslib/conduit/reflect/schema/columns.hpp>
#include <erslib/conduit/reflect/schema/members.hpp>
#include <erslib/conduit/reflect/types.hpp>


namespace conduit::impl::reflect {
    template<typename T>
    consteval std::span<const char* const> collect_primary_key() {
        std::vector<const char*> out;

        template for (constexpr auto it : declaration_list<T>) {
            using entry = entry_type<it>;

            if constexpr (EntryOf<entry, kinds::EPk>)
                for (const std::string_view name : entry_traits<entry>::columns)
                    out.emplace_back(std::define_static_string(name));
        }

        return std::define_static_array(out);
    }

    template<typename T>
    constexpr std::span<const char* const> primary_key = collect_primary_key<T>();


    template<typename T>
    consteval std::string_view find_pk_constraint() {
        template for (constexpr auto it : declaration_list<T>) {
            using entry = entry_type<it>;

            if constexpr (EntryOf<entry, kinds::EPk>)
                return std::define_static_string(std::meta::identifier_of(it));
        }

        return {};
    }

    template<typename T>
    constexpr std::string_view pk_constraint = find_pk_constraint<T>();


    template<typename T, std::meta::info M>
    consteval bool check_pk_column() {
        for (const char* const it : primary_key<T>) {
            if (std::string_view(it) == column_name<M>)
                return true;
        }

        return false;
    }

    template<typename T, std::meta::info M>
    constexpr bool is_pk_column = check_pk_column<T, M>();


    template<typename T, std::meta::info M>
    consteval bool check_identity_column() {
        template for (constexpr auto it : declaration_list<T>) {
            using entry = entry_type<it>;

            if constexpr (EntryOf<entry, kinds::EIdentity>)
                if constexpr (entry_traits<entry>::field == column_name<M>)
                    return true;
        }

        if (primary_key<T>.size() != 1 || std::string_view(primary_key<T>.front()) != column_name<M>)
            return false;

        return std::is_integral_v<member_type<M>>;
    }

    template<typename T, std::meta::info M>
    constexpr bool is_identity_column = check_identity_column<T, M>();


    template<typename T>
    consteval std::string_view find_identity_column() {
        template for (constexpr auto m : column_list<T>) {
            if constexpr (is_identity_column<T, m>)
                return column_name<m>;
        }

        return {};
    }

    template<typename T>
    constexpr std::string_view identity_column = find_identity_column<T>();


    template<typename T>
    constexpr bool has_identity = !identity_column<T>.empty();
}
