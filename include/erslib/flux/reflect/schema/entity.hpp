#pragma once

// std
#include <meta>
#include <string_view>
#include <type_traits>

// ers
#include <erslib/flux/reflect/declaration.hpp>
#include <erslib/flux/reflect/types.hpp>


namespace flux::impl::reflect {
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
    consteval std::meta::info table_schema() {
        template for (constexpr auto it : std::define_static_array(std::meta::bases_of(^^Declaration<T>, std::meta::access_context::current()))) {
            using base = typename [:std::meta::type_of(it):];

            if constexpr (EntryOf<base, kinds::ETable>)
                return std::meta::dealias(^^typename base::schema);
        }

        return ^^NoSchema;
    }

    template<typename T>
    constexpr std::string_view table_name = base_tag<T, kinds::ETable>();

    template<typename T>
    constexpr std::string_view query_label = base_tag<T, kinds::EQuery>();

    template<typename T>
    using schema_of = typename [:table_schema<T>():];

    template<typename T>
    constexpr std::string_view schema_name = schema_of<T>::name;

    template<typename T>
    constexpr std::string_view qualified_name = std::define_static_string(qualify(schema_name<T>, table_name<T>));

    template<typename T>
    concept RowType = std::is_class_v<T> && requires { sizeof(Declaration<T>); };

    template<typename T>
    concept Entity = RowType<T> && !table_name<T>.empty();

    template<typename T>
    concept Statement = RowType<T> && !query_label<T>.empty();
}
