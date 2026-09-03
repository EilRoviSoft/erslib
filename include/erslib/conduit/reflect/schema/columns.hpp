#pragma once

// std
#include <meta>
#include <string_view>

// ers
#include <erslib/conduit/traits.hpp>
#include <erslib/conduit/reflect/schema/members.hpp>
#include <erslib/conduit/reflect/types.hpp>


namespace conduit::impl::reflect {
    template<std::meta::info M>
    using member_type = typename [:std::meta::dealias(std::meta::remove_cv(std::meta::type_of(M))):];


    template<std::meta::info M>
    constexpr std::string_view column_name = [] consteval -> std::string_view {
        template for (constexpr auto it : std::define_static_array(std::meta::annotations_of(M))) {
            using entry = typename [:std::meta::remove_cv(std::meta::type_of(it)):];

            if constexpr (EntryOf<entry, kinds::column>)
                if constexpr (!entry::name.empty())
                    return entry::name;
        }

        return std::define_static_string(std::meta::identifier_of(M));
    }();

    template<std::meta::info M>
    constexpr std::string_view sql_type_name = [] consteval -> std::string_view {
        template for (constexpr auto it : std::define_static_array(std::meta::annotations_of(M))) {
            using entry = typename [:std::meta::remove_cv(std::meta::type_of(it)):];

            if constexpr (EntryOf<entry, kinds::column>)
                if constexpr (!entry::type.empty())
                    return entry::type;
        }

        return sql_type<member_type<M>>::name;
    }();

    template<std::meta::info M>
    constexpr bool is_nullable = sql_type<member_type<M>>::nullable;


    template<typename T>
    consteval bool has_field(std::string_view what) {
        for (const auto it : all_members<T>()) {
            if (std::meta::identifier_of(it) == what)
                return true;
        }

        return false;
    }

    template<typename T>
    consteval bool has_column(std::string_view what) {
        template for (constexpr auto it : column_list<T>) {
            if (column_name<it> == what)
                return true;
        }

        return false;
    }
}
