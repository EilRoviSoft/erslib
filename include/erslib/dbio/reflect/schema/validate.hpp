#pragma once

// std
#include <meta>
#include <string_view>

// ers
#include <erslib/dbio/reflect/schema/columns.hpp>
#include <erslib/dbio/reflect/schema/members.hpp>
#include <erslib/dbio/reflect/types.hpp>


namespace dbio::impl::reflect {
    template<typename T>
    constexpr bool declaration_is_valid = [] consteval {
        template for (constexpr auto it : declaration_list<T>) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (EntryOf<entry, kinds::pk>) {
                for (const std::string_view name : entry::names) {
                    if (!has_column<T>(name))
                        return false;
                }
            } else if constexpr (EntryOf<entry, kinds::unique>) {
                for (const std::string_view name : entry::names) {
                    if (!has_column<T>(name))
                        return false;
                }
            } else if constexpr (EntryOf<entry, kinds::fk>) {
                if (!has_column<T>(entry::field))
                    return false;
            } else if constexpr (EntryOf<entry, kinds::def>) {
                if (!has_column<T>(entry::field))
                    return false;
            } else if constexpr (EntryOf<entry, kinds::identity>) {
                if (!has_column<T>(entry::field))
                    return false;
            }
        }

        return true;
    }();
}
