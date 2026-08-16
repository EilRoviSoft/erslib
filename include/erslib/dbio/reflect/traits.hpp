#pragma once

#ifndef ERSLIB_HAS_REFLECTION
#  error "erslib/dbio/reflect requires C++26 static reflection (configure with ERSLIB_ENABLE_REFLECTION=ON)"
#endif

// std
#include <meta>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

// ers
#include <erslib/dbio/reflect/annotations.hpp>
#include <erslib/dbio/reflect/definition.hpp>
#include <erslib/dbio/reflect/sql_type.hpp>


// Annotation access

namespace dbio::impl {
    template<typename T>
    consteval bool has(std::meta::info what) {
        for (const auto it : std::meta::annotations_of(what))
            if (std::meta::remove_cv(std::meta::type_of(it)) == ^^T)
                return true;

        return false;
    }

    template<std::meta::info R, template<ers::fixed_string> class A>
    consteval bool has_text() {
        template for (constexpr auto it : std::define_static_array(std::meta::annotations_of(R))) {
            constexpr auto type = std::meta::remove_cv(std::meta::type_of(it));

            if constexpr (std::meta::has_template_arguments(type) && std::meta::template_of(type) == ^^A)
                return true;
        }

        return false;
    }

    template<std::meta::info R, template<ers::fixed_string> class A>
    consteval std::string_view text_of() {
        template for (constexpr auto it : std::define_static_array(std::meta::annotations_of(R))) {
            constexpr auto type = std::meta::remove_cv(std::meta::type_of(it));

            if constexpr (std::meta::has_template_arguments(type) && std::meta::template_of(type) == ^^A)
                return [:type:]::value;
        }

        return {};
    }

    template<std::meta::info R>
    consteval bool has_fk() {
        template for (constexpr auto it : std::define_static_array(std::meta::annotations_of(R))) {
            constexpr auto type = std::meta::remove_cv(std::meta::type_of(it));

            if constexpr (std::meta::has_template_arguments(type) && std::meta::template_of(type) == ^^Fk)
                return true;
        }

        return false;
    }
}


// Definition shape

namespace dbio::impl {
    consteval bool is_metadata(std::meta::info m) {
        const std::meta::info type = std::meta::dealias(std::meta::type_of(m));

        return std::meta::has_template_arguments(type)
            && std::meta::template_of(type) == ^^Unique;
    }


    template<typename T>
    consteval std::vector<std::meta::info> columns() {
        std::vector<std::meta::info> out;

        for (const auto it : nonstatic_data_members_of(^^Definition<T>,
                 std::meta::access_context::current()))
            if (!is_metadata(it) && !has<Skip>(it))
                out.emplace_back(it);

        return out;
    }

    template<typename T>
    consteval std::vector<std::meta::info> entity_members() {
        return nonstatic_data_members_of(^^T, std::meta::access_context::current());
    }

    template<typename T>
    consteval std::string_view table_name() {
        template for (constexpr auto it : std::define_static_array(
            std::meta::bases_of(^^Definition<T>, std::meta::access_context::current()))) {
            constexpr auto type = std::meta::dealias(std::meta::type_of(it));

            if constexpr (std::meta::has_template_arguments(type) && std::meta::template_of(type) == ^^Table)
                return [:type:]::table;
        }

        return {};
    }

    template<typename T>
    concept Entity = std::is_class_v<T> && !table_name<T>().empty();


    template<std::meta::info M>
    consteval std::string_view column_name() {
        if constexpr (has_text<M, Name>())
            return std::define_static_string(text_of<M, Name>());
        else
            return std::define_static_string(std::meta::identifier_of(M));
    }

    // NOT NULL is the default; only an optional-like type or Nullable opts out.
    template<std::meta::info M>
    consteval bool is_nullable_column() {
        if constexpr (has<Nullable>(M)) {
            return true;
        } else {
            using type = [:std::meta::dealias(std::meta::remove_cv(std::meta::type_of(M))):];
            return dbio::reflect::is_nullable<type>();
        }
    }

    template<std::meta::info M>
    consteval std::string_view sql_type_of() {
        if constexpr (has_text<M, Column>()) {
            return std::define_static_string(text_of<M, Column>());
        } else {
            using type = [:std::meta::dealias(std::meta::remove_cv(std::meta::type_of(M))):];

            static_assert(dbio::reflect::HasSqlType<type>,
                "no SQL mapping for this member's type; specialize "
                "dbio::reflect::sql_type_traits<T> or annotate the member with Column<\"...\">");

            return dbio::reflect::sql_type_traits<type>::value;
        }
    }
}


// Keys

namespace dbio::impl {
    template<typename T>
    consteval std::span<const char* const> column_names() {
        std::vector<const char*> out;

        template for (constexpr auto it : std::define_static_array(columns<T>()))
            out.emplace_back(std::define_static_string(column_name<it>()));

        return std::define_static_array(out);
    }

    template<typename T>
    consteval std::span<const char* const> primary_key() {
        std::vector<const char*> out;

        template for (constexpr auto it : std::define_static_array(columns<T>()))
            if constexpr (has<Pk>(it))
                out.emplace_back(std::define_static_string(column_name<it>()));

        return std::define_static_array(out);
    }

    // Pk implies identity only for a sole integral key.
    template<std::meta::info M, typename T>
    consteval bool is_identity() {
        if constexpr (has<Identity>(M))
            return true;
        else if constexpr (!has<Pk>(M))
            return false;
        else
            return primary_key<T>().size() == 1
                && std::is_integral_v<typename [:std::meta::dealias(std::meta::type_of(M)):]>;
    }


    template<typename T>
    consteval std::vector<std::vector<const char*>> unique_groups() {
        std::vector<std::vector<const char*>> out;

        template for (constexpr auto it : std::define_static_array(
            nonstatic_data_members_of(^^Definition<T>, std::meta::access_context::current()))) {
            constexpr auto type = std::meta::dealias(std::meta::type_of(it));

            if constexpr (std::meta::has_template_arguments(type) && std::meta::template_of(type) == ^^Unique) {
                std::vector<const char*> group;

                for (const std::string_view name : [:type:]::fields)
                    group.emplace_back(std::define_static_string(name));

                out.emplace_back(std::move(group));
            }
        }

        return out;
    }

    template<typename T>
    consteval size_t column_count() {
        return columns<T>().size();
    }

    template<typename T>
    consteval size_t unique_count() {
        return unique_groups<T>().size();
    }
}
