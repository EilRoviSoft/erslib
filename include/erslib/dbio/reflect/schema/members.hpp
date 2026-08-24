#pragma once

// std
#include <meta>
#include <vector>

// ers
#include <erslib/dbio/reflect/declaration.hpp>
#include <erslib/dbio/reflect/types.hpp>


namespace dbio::impl::reflect {
    template<typename T>
    consteval std::vector<std::meta::info> all_members() {
        return nonstatic_data_members_of(^^T, std::meta::access_context::current());
    }


    template<typename T>
    consteval std::vector<std::meta::info> declaration_members() {
        return nonstatic_data_members_of(^^Declaration<T>, std::meta::access_context::current());
    }


    consteval bool is_skipped(std::meta::info member) {
        for (const auto it : std::meta::annotations_of(member)) {
            if (std::meta::remove_cv(std::meta::type_of(it)) == ^^skip_t)
                return true;
        }

        return false;
    }

    template<typename T>
    consteval std::vector<std::meta::info> columns() {
        std::vector<std::meta::info> out;

        for (const auto it : all_members<T>()) {
            if (!is_skipped(it))
                out.emplace_back(it);
        }

        return out;
    }

    template<typename T>
    constexpr auto column_list = std::define_static_array(columns<T>());

    template<typename T>
    constexpr auto declaration_list = std::define_static_array(declaration_members<T>());

    template<typename T>
    constexpr size_t column_count = column_list<T>.size();
}
