#pragma once

// std
#include <meta>
#include <string_view>
#include <type_traits>

// ers
#include <erslib/dbio/reflect/declaration.hpp>
#include <erslib/dbio/reflect/types.hpp>


namespace dbio::impl::reflect {
    template<typename T, typename Kind>
    consteval std::string_view base_tag() {
        template for (constexpr auto it : std::define_static_array(std::meta::bases_of(^^Declaration<T>, std::meta::access_context::current()))) {
            using base = typename [:std::meta::type_of(it):];

            if constexpr (EntryOf<base, Kind>)
                return base::name;
        }

        return {};
    }

    template<typename T>
    constexpr std::string_view table_name = base_tag<T, kinds::table>();

    template<typename T>
    constexpr std::string_view query_label = base_tag<T, kinds::query>();

    template<typename T>
    concept RowType = std::is_class_v<T> && requires { sizeof(Declaration<T>); };

    template<typename T>
    concept Entity = RowType<T> && !table_name<T>.empty();

    template<typename T>
    concept Statement = RowType<T> && !query_label<T>.empty();
}
