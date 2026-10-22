#pragma once

// std
#include <meta>
#include <string_view>
#include <vector>

// ers
#include <erslib/flux/reflect/schema/columns.hpp>
#include <erslib/flux/reflect/schema/keys.hpp>
#include <erslib/flux/reflect/schema/members.hpp>
#include <erslib/flux/reflect/types.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::reflect {
    // The column marked by SoftDelete, empty if there is none.
    template<typename T>
    consteval std::string_view find_soft_delete_column() {
        template for (constexpr auto it : metadata_list<T>) {
            using entry = entry_type<it>;

            if constexpr (EntryOf<entry, kinds::ESoftDelete>)
                return entry_traits<entry>::field;
        }

        return {};
    }

    template<typename T>
    constexpr std::string_view soft_delete_column = find_soft_delete_column<T>();

    template<typename T>
    constexpr bool has_soft_delete = !soft_delete_column<T>.empty();

    template<typename T>
    consteval bool soft_delete_is_nullable() {
        template for (constexpr auto m : column_list<T>) {
            if constexpr (column_name<m> == soft_delete_column<T>)
                return is_nullable<m>;
        }

        return true;
    }


    // CRUD never writes these: the database fills ReadOnly, erase and restore set SoftDelete.
    template<typename T, std::meta::info M>
    consteval bool check_read_only_column() {
        template for (constexpr auto it : metadata_list<T>) {
            using entry = entry_type<it>;

            if constexpr (EntryOf<entry, kinds::EReadOnly> || EntryOf<entry, kinds::ESoftDelete>)
                if constexpr (entry_traits<entry>::field == column_name<M>)
                    return true;
        }

        return false;
    }

    template<typename T, std::meta::info M>
    constexpr bool is_read_only_column = check_read_only_column<T, M>();


    template<typename T>
    consteval bool validate_metadata() {
        template for (constexpr auto it : metadata_list<T>) {
            for (const std::string_view column : entry_traits<entry_type<it>>::columns) {
                if (!has_column<T>(column))
                    return false;
            }
        }

        return true;
    }

    template<typename T>
    constexpr bool metadata_is_valid = validate_metadata<T>();


    template<typename T>
    consteval std::vector<std::meta::info> collect_insert_members() {
        std::vector<std::meta::info> out;

        template for (constexpr auto m : column_list<T>) {
            if constexpr (!is_identity_column<T, m> && !is_read_only_column<T, m>)
                out.emplace_back(m);
        }

        return out;
    }

    template<typename T>
    consteval std::vector<std::meta::info> collect_update_members() {
        std::vector<std::meta::info> out;

        template for (constexpr auto m : column_list<T>) {
            if constexpr (!is_identity_column<T, m> && !is_read_only_column<T, m> && !is_pk_column<T, m>)
                out.emplace_back(m);
        }

        return out;
    }

    template<typename T>
    constexpr auto insert_members = std::define_static_array(collect_insert_members<T>());

    template<typename T>
    constexpr auto update_members = std::define_static_array(collect_update_members<T>());


    template<typename T, std::meta::info M>
    consteval bool check_update_member() {
        for (const auto it : update_members<T>) {
            if (it == M)
                return true;
        }

        return false;
    }

    template<typename T, std::meta::info M>
    constexpr bool is_update_member = check_update_member<T, M>();


    template<typename T>
    consteval bool has_unique_constraint(std::string_view name) {
        template for (constexpr auto it : declaration_list<T>) {
            using entry = entry_type<it>;

            if constexpr (EntryOf<entry, kinds::EPk> || EntryOf<entry, kinds::EUnique>)
                if (std::meta::identifier_of(it) == name)
                    return true;
        }

        return false;
    }
}
