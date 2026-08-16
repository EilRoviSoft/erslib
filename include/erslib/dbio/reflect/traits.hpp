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

        if (type == ^^statement::SelectAll || type == ^^statement::Save
            || type == ^^statement::Update || type == ^^statement::Delete)
            return true;

        if (std::meta::has_template_arguments(type)) {
            const std::meta::info tmpl = std::meta::template_of(type);

            return tmpl == ^^Unique
                || tmpl == ^^statement::LoadBy
                || tmpl == ^^statement::DeleteBy;
        }

        return false;
    }

    consteval bool is_statement(std::meta::info m) {
        const std::meta::info type = std::meta::dealias(std::meta::type_of(m));

        if (type == ^^statement::SelectAll || type == ^^statement::Save
            || type == ^^statement::Update || type == ^^statement::Delete)
            return true;

        if (std::meta::has_template_arguments(type)) {
            const std::meta::info tmpl = std::meta::template_of(type);
            return tmpl == ^^statement::LoadBy || tmpl == ^^statement::DeleteBy;
        }

        return false;
    }


    template<typename T>
    consteval std::vector<std::meta::info> columns() {
        std::vector<std::meta::info> out;

        for (const auto it : nonstatic_data_members_of(^^dbio::Definition<T>,
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
    consteval std::vector<std::meta::info> statements() {
        std::vector<std::meta::info> out;

        for (const auto it : nonstatic_data_members_of(^^dbio::Definition<T>,
                std::meta::access_context::current()))
            if (is_statement(it))
                out.emplace_back(it);

        return out;
    }


    template<typename T>
    consteval std::string_view table_name() {
        template for (constexpr auto it : std::define_static_array(
            std::meta::bases_of(^^dbio::Definition<T>, std::meta::access_context::current()))) {
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
                nonstatic_data_members_of(^^dbio::Definition<T>, std::meta::access_context::current()))) {
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

    // First UNIQUE group, else the primary key.
    template<typename T>
    consteval std::span<const char* const> conflict_target() {
        const auto groups = unique_groups<T>();

        if (groups.empty())
            return primary_key<T>();

        return std::define_static_array(groups.front());
    }

    template<typename T>
    consteval size_t column_count() {
        return columns<T>().size();
    }

    template<typename T>
    consteval size_t statement_count() {
        return statements<T>().size();
    }

    template<typename T>
    consteval size_t unique_count() {
        return unique_groups<T>().size();
    }
}


// Roles

namespace dbio::impl {
    enum class Role : uint8_t {
        None, In, Out, Key
    };

    // Fields a LoadBy/DeleteBy statement matches on.
    template<std::meta::info S>
    consteval std::vector<const char*> statement_keys() {
        std::vector<const char*> out;
        constexpr auto type = std::meta::dealias(std::meta::type_of(S));

        if constexpr (std::meta::has_template_arguments(type)
            && (std::meta::template_of(type) == ^^statement::LoadBy
                || std::meta::template_of(type) == ^^statement::DeleteBy))
            for (const std::string_view name : [:type:]::fields)
                out.emplace_back(std::define_static_string(name));

        return out;
    }

    template<typename T, std::meta::info S, Role R>
    consteval std::span<const char* const> role_columns() {
        std::vector<const char*> out;

        constexpr auto type = std::meta::dealias(std::meta::type_of(S));
        const auto keys = statement_keys<S>();

        const auto listed = [&keys](std::string_view what) consteval {
            for (const char* it : keys)
                if (std::string_view(it) == what)
                    return true;

            return false;
        };

        template for (constexpr auto it : std::define_static_array(columns<T>())) {
            constexpr std::string_view name = column_name<it>();
            constexpr bool pk = has<Pk>(it);

            Role role = Role::None;

            if constexpr (type == ^^statement::SelectAll)
                role = Role::In;
            else if constexpr (type == ^^statement::Save)
                role = pk ? Role::In : Role::Out;
            else if constexpr (type == ^^statement::Update)
                role = pk ? Role::Key : Role::Out;
            else if constexpr (type == ^^statement::Delete)
                role = pk ? Role::Key : Role::None;
            else
                role = listed(name) ? Role::Key : Role::In;

            if (role == R)
                out.emplace_back(std::define_static_string(name));
        }

        return std::define_static_array(out);
    }

    // Written by the layout, minus the conflict target and primary key.
    template<typename T, std::meta::info S>
    consteval std::span<const char* const> upsert_updates() {
        std::vector<const char*> out;

        constexpr auto target = conflict_target<T>();
        constexpr auto primary = primary_key<T>();

        const auto listed = [](std::span<const char* const> where, std::string_view what) consteval {
            for (const char* it : where)
                if (std::string_view(it) == what)
                    return true;

            return false;
        };

        for (auto it : role_columns<T, S, Role::Out>())
            if (!listed(target, it) && !listed(primary, it))
                out.emplace_back(it);

        return std::define_static_array(out);
    }
}


// Exports

namespace dbio::reflect {
    using impl::Entity;
    using impl::Role;

    using impl::has;
    using impl::has_text;
    using impl::text_of;

    using impl::columns;
    using impl::statements;
    using impl::entity_members;
    using impl::column_count;
    using impl::statement_count;
    using impl::unique_count;
    using impl::column_names;
    using impl::column_name;
    using impl::sql_type_of;

    using impl::table_name;
    using impl::primary_key;
    using impl::unique_groups;
    using impl::conflict_target;

    using impl::role_columns;
    using impl::statement_keys;
    using impl::upsert_updates;
}
