#pragma once

// std
#include <meta>
#include <string_view>

// ers
#include <erslib/conduit/reflect/schema/columns.hpp>
#include <erslib/conduit/reflect/schema/members.hpp>
#include <erslib/conduit/reflect/types.hpp>


namespace conduit::impl::reflect {
    template<typename T>
    consteval bool validate_declaration() {
        template for (constexpr auto it : declaration_list<T>) {
            for (const std::string_view column : entry_traits<entry_type<it>>::columns) {
                if (!has_column<T>(column))
                    return false;
            }
        }

        return true;
    }

    template<typename T>
    constexpr bool declaration_is_valid = validate_declaration<T>();
}
