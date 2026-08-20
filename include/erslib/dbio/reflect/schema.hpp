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

namespace dbio::impl {
    template<typename T>
    consteval std::vector<std::meta::info> all_members() {
        return nonstatic_data_members_of(^^T, std::meta::access_context::current());
    }

    template<typename T>
    consteval std::vector<std::meta::info> declaration_members() {
        return nonstatic_data_members_of(^^Declaration<T>,
            std::meta::access_context::current());
    }

    // Column and Skip are keyed by the declaration member's own name.
    template<typename T>
    consteval bool is_skipped(std::string_view field) {
        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_skip<entry>::value)
                if (std::meta::identifier_of(it) == field)
                    return true;
        }

        return false;
    }

    template<typename T>
    consteval std::vector<std::meta::info> columns() {
        std::vector<std::meta::info> out;

        for (const auto it : all_members<T>())
            if (!is_skipped<T>(std::meta::identifier_of(it)))
                out.emplace_back(it);

        return out;
    }

    template<typename T>
    consteval std::string_view table_name() {
        template for (constexpr auto it : std::define_static_array(
            std::meta::bases_of(^^Declaration<T>, std::meta::access_context::current()))) {
            using base = typename [:std::meta::type_of(it):];

            if constexpr (is_table<base>::value)
                return std::define_static_string(base::table_name.to_sv());
        }

        return {};
    }

    template<typename T>
    concept Entity = std::is_class_v<T> && !table_name<T>().empty();
}


// Columns

namespace dbio::impl {
    template<std::meta::info M>
    using member_type = typename [:std::meta::dealias(std::meta::remove_cv(std::meta::type_of(M))):];


    template<typename T, std::meta::info M>
    consteval std::string_view column_name() {
        constexpr std::string_view field = std::define_static_string(std::meta::identifier_of(M));

        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_column<entry>::value)
                if constexpr (std::define_static_string(std::meta::identifier_of(it)) == field)
                    if constexpr (!column_info<entry>::name.empty())
                        return column_info<entry>::name;
        }

        return field;
    }

    template<typename T, std::meta::info M>
    consteval std::string_view sql_type_name() {
        constexpr std::string_view field = std::define_static_string(std::meta::identifier_of(M));

        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_column<entry>::value)
                if constexpr (std::define_static_string(std::meta::identifier_of(it)) == field)
                    if constexpr (!column_info<entry>::type.empty())
                        return column_info<entry>::type;
        }

        return sql_type<member_type<M>>::name;
    }

    template<std::meta::info M>
    consteval bool is_nullable() {
        return sql_type<member_type<M>>::nullable;
    }


    template<typename T>
    consteval bool has_field(std::string_view what) {
        for (const auto it : all_members<T>())
            if (std::meta::identifier_of(it) == what)
                return true;

        return false;
    }

    template<typename T>
    consteval bool has_column(std::string_view what) {
        template for (constexpr auto it : std::define_static_array(columns<T>()))
            if (column_name<T, it>() == what)
                return true;

        return false;
    }
}


// Column facts drawn from the declaration

namespace dbio::impl {
    template<typename T>
    consteval std::span<const char* const> primary_key() {
        std::vector<const char*> out;

        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_pk<entry>::value)
                for (const std::string_view name : pk_fields<entry>::names)
                    out.emplace_back(std::define_static_string(name));
        }

        return std::define_static_array(out);
    }

    template<typename T, std::meta::info M>
    consteval bool is_identity_column() {
        constexpr std::string_view name = column_name<T, M>();

        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_identity<entry>::value)
                if constexpr (identity_info<entry>::field == name)
                    return true;
        }

        constexpr auto key = primary_key<T>();

        if (key.size() != 1 || std::string_view(key.front()) != name)
            return false;

        return std::is_integral_v<member_type<M>>;
    }
}


// Constraints named by the declaration

namespace dbio::impl {
    template<typename T>
    consteval size_t column_count() {
        return columns<T>().size();
    }

    template<typename T, std::meta::info M>
    consteval bool is_pk_column() {
        constexpr std::string_view name = column_name<T, M>();

        for (const char* const it : primary_key<T>())
            if (std::string_view(it) == name)
                return true;

        return false;
    }

    template<typename T>
    consteval bool has_identity() {
        template for (constexpr auto m : std::define_static_array(columns<T>()))
            if constexpr (is_identity_column<T, m>())
                return true;

        return false;
    }

    template<typename T>
    consteval std::string_view identity_column() {
        template for (constexpr auto m : std::define_static_array(columns<T>()))
            if constexpr (is_identity_column<T, m>())
                return column_name<T, m>();

        return {};
    }


    template<typename T>
    consteval std::string_view pk_constraint() {
        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_pk<entry>::value)
                return std::define_static_string(std::meta::identifier_of(it));
        }

        return {};
    }

    template<typename T>
    consteval size_t unique_count() {
        size_t out = 0;

        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_unique<entry>::value)
                out++;
        }

        return out;
    }

    template<typename T>
    consteval std::string_view sole_unique_constraint() {
        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_unique<entry>::value)
                return std::define_static_string(std::meta::identifier_of(it));
        }

        return {};
    }


    // Names of a Pk or Unique member, looked up by the member's own name.
    template<typename T>
    consteval bool is_constraint_name(std::string_view what) {
        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_pk<entry>::value || is_unique<entry>::value)
                if (std::meta::identifier_of(it) == what)
                    return true;
        }

        return false;
    }

    template<typename T>
    consteval std::span<const char* const> constraint_columns(std::string_view what) {
        std::vector<const char*> out;

        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_pk<entry>::value) {
                if (std::meta::identifier_of(it) == what)
                    for (const std::string_view name : pk_fields<entry>::names)
                        out.emplace_back(std::define_static_string(name));
            } else if constexpr (is_unique<entry>::value) {
                if (std::meta::identifier_of(it) == what)
                    for (const std::string_view name : unique_fields<entry>::names)
                        out.emplace_back(std::define_static_string(name));
            }
        }

        return std::define_static_array(out);
    }
}


// The constraint save() upserts on

namespace dbio::impl {
    template<typename T>
    consteval std::string_view declared_conflict() {
        template for (constexpr auto it : std::define_static_array(declaration_members<T>())) {
            using entry = typename [:std::meta::type_of(it):];

            if constexpr (is_conflict<entry>::value)
                return conflict_info<entry>::constraint;
        }

        return {};
    }

    template<typename T>
    consteval std::string_view conflict_constraint() {
        constexpr std::string_view declared = declared_conflict<T>();

        if constexpr (!declared.empty())
            return declared;
        else if constexpr (!has_identity<T>())
            return pk_constraint<T>();
        else if constexpr (unique_count<T>() == 1)
            return sole_unique_constraint<T>();
        else
            return {};
    }

    template<typename T>
    consteval std::span<const char* const> conflict_columns() {
        constexpr std::string_view name = conflict_constraint<T>();

        if constexpr (name.empty())
            return {};
        else
            return constraint_columns<T>(name);
    }

    template<typename T, std::meta::info M>
    consteval bool is_conflict_column() {
        constexpr std::string_view name = column_name<T, M>();

        for (const char* const it : conflict_columns<T>())
            if (std::string_view(it) == name)
                return true;

        return false;
    }
}


// Validation

namespace dbio::impl {
    template<typename T>
    consteval bool declaration_is_valid() {
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
            } else if constexpr (is_conflict<entry>::value) {
                if (!is_constraint_name<T>(conflict_info<entry>::constraint))
                    return false;
            } else if constexpr (is_column<entry>::value || is_skip<entry>::value) {
                if (!has_field<T>(std::meta::identifier_of(it)))
                    return false;
            }
        }

        return true;
    }
}
