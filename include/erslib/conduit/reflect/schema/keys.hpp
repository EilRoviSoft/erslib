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
    constexpr std::span<const char* const> primary_key = [] consteval {
        std::vector<const char*> out;

        template for (constexpr auto it : declaration_list<T>) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (EntryOf<entry, kinds::pk>)
                for (const std::string_view name : entry::names)
                    out.emplace_back(std::define_static_string(name));
        }

        return std::span(std::define_static_array(out));
    }();

    template<typename T>
    constexpr std::string_view pk_constraint = [] consteval -> std::string_view {
        template for (constexpr auto it : declaration_list<T>) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (EntryOf<entry, kinds::pk>)
                return std::define_static_string(std::meta::identifier_of(it));
        }

        return {};
    }();

    template<typename T, std::meta::info M>
    constexpr bool is_pk_column = [] consteval {
        for (const char* const it : primary_key<T>) {
            if (std::string_view(it) == column_name<M>)
                return true;
        }

        return false;
    }();

    template<typename T, std::meta::info M>
    constexpr bool is_identity_column = [] consteval {
        template for (constexpr auto it : declaration_list<T>) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (EntryOf<entry, kinds::identity>)
                if constexpr (entry::field == column_name<M>)
                    return true;
        }

        if (primary_key<T>.size() != 1 || std::string_view(primary_key<T>.front()) != column_name<M>)
            return false;

        return std::is_integral_v<member_type<M>>;
    }();

    template<typename T>
    constexpr bool has_identity = [] consteval {
        template for (constexpr auto m : column_list<T>) {
            if constexpr (is_identity_column<T, m>)
                return true;
        }

        return false;
    }();

    template<typename T>
    constexpr std::string_view identity_column = [] consteval -> std::string_view {
        template for (constexpr auto m : column_list<T>) {
            if constexpr (is_identity_column<T, m>)
                return column_name<m>;
        }

        return {};
    }();
}
