#pragma once

// std
#include <meta>
#include <span>
#include <string_view>
#include <type_traits>
#include <vector>

// ers
#include <erslib/dbio/reflect/declaration.hpp>
#include <erslib/dbio/reflect/traits.hpp>
#include <erslib/dbio/reflect/types.hpp>


// Shape

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
    constexpr size_t column_count = columns<T>().size();

    template<typename T, template<typename> typename IsKind>
    consteval std::string_view base_tag() {
        template for (constexpr auto it : std::define_static_array(std::meta::bases_of(^^Declaration<T>, std::meta::access_context::current()))) {
            using base = typename [:std::meta::type_of(it):];

            if constexpr (IsKind<base>::value)
                return std::define_static_string(base::name.to_sv());
        }

        return {};
    }

    template<typename T>
    constexpr std::string_view table_name = base_tag<T, is_table>();

    template<typename T>
    constexpr std::string_view query_label = base_tag<T, is_query>();

    template<typename T>
    concept RowType = std::is_class_v<T> && requires { sizeof(Declaration<T>); };

    template<typename T>
    concept Entity = RowType<T> && !table_name<T>.empty();

    template<typename T>
    concept Statement = RowType<T> && !query_label<T>.empty();
}


// Columns

namespace dbio::impl::reflect {
    template<std::meta::info M>
    using member_type = typename [:std::meta::dealias(std::meta::remove_cv(std::meta::type_of(M))):];


    template<std::meta::info M>
    constexpr std::string_view column_name = [] consteval -> std::string_view {
        template for (constexpr auto it : std::define_static_array(std::meta::annotations_of(M))) {
            using entry = typename [:std::meta::remove_cv(std::meta::type_of(it)):];

            if constexpr (is_column<entry>::value)
                if constexpr (!column_info<entry>::name.empty())
                    return column_info<entry>::name;
        }

        return std::define_static_string(std::meta::identifier_of(M));
    }();

    template<std::meta::info M>
    constexpr std::string_view sql_type_name = [] consteval -> std::string_view {
        template for (constexpr auto it : std::define_static_array(std::meta::annotations_of(M))) {
            using entry = typename [:std::meta::remove_cv(std::meta::type_of(it)):];

            if constexpr (is_column<entry>::value)
                if constexpr (!column_info<entry>::type.empty())
                    return column_info<entry>::type;
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
        template for (constexpr auto it : std::define_static_array(columns<T>())) {
            if (column_name<it> == what)
                return true;
        }

        return false;
    }
}


// Column info got from `Declaration<T>`

namespace dbio::impl::reflect {
    template<typename T>
    constexpr std::span<const char* const> primary_key = [] consteval {
        std::vector<const char*> out;

        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_pk<entry>::value)
                for (const std::string_view name : pk_fields<entry>::names)
                    out.emplace_back(std::define_static_string(name));
        }

        return std::span(std::define_static_array(out));
    }();

    template<typename T>
    constexpr std::string_view pk_constraint = [] consteval -> std::string_view {
        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_pk<entry>::value)
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
        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_identity<entry>::value)
                if constexpr (identity_info<entry>::field == column_name<M>)
                    return true;
        }

        if (primary_key<T>.size() != 1 || std::string_view(primary_key<T>.front()) != column_name<M>)
            return false;

        return std::is_integral_v<member_type<M>>;
    }();

    template<typename T>
    constexpr bool has_identity = [] consteval {
        template for (constexpr auto m : std::define_static_array(columns<T>())) {
            if constexpr (is_identity_column<T, m>)
                return true;
        }

        return false;
    }();

    template<typename T>
    constexpr std::string_view identity_column = [] consteval -> std::string_view {
        template for (constexpr auto m : std::define_static_array(columns<T>())) {
            if constexpr (is_identity_column<T, m>)
                return column_name<m>;
        }

        return {};
    }();
}


// Validation

namespace dbio::impl::reflect {
    template<typename T>
    constexpr bool declaration_is_valid = [] consteval {
        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_pk<entry>::value) {
                for (const std::string_view name : pk_fields<entry>::names)
                    if (!has_column<T>(name))
                        return false;
            } else if constexpr (is_unique<entry>::value) {
                for (const std::string_view name : unique_fields<entry>::names)
                    if (!has_column<T>(name))
                        return false;
            } else if constexpr (is_fk<entry>::value) {
                if (!has_column<T>(fk_info<entry>::field))
                    return false;
            } else if constexpr (is_default<entry>::value) {
                if (!has_column<T>(default_info<entry>::field))
                    return false;
            } else if constexpr (is_identity<entry>::value) {
                if (!has_column<T>(identity_info<entry>::field))
                    return false;
            }
        }

        return true;
    }();
}
