#pragma once

// std
#include <string>
#include <vector>

// ers
#include <erslib/dbio/constant.hpp>
#include <erslib/dbio/eval.hpp>
#include <erslib/dbio/pqxx_row.hpp>
#include <erslib/dbio/query.hpp>
#include <erslib/dbio/reflect/traits.hpp>


// Internal

namespace dbio::impl {
    inline constexpr int placeholder = 0;

    inline Clause dummy_values(size_t count) {
        std::vector<binder_t> out;
        out.reserve(count);

        for (size_t i = 0; i < count; i++)
            out.emplace_back(make_binder(placeholder));

        return make_clause<ValuesClause>(std::move(out));
    }
}


// SQL construction

namespace dbio::impl {
    template<Entity T, std::meta::info S>
    ers::Result<std::string> build_select() {
        ERS_QUICK_DBIO_USING;

        auto query = select_from(std::string(table_name<T>()))
            | columns(role_columns<T, S, Role::In>());

        for (const char* it : role_columns<T, S, Role::Key>())
            query |= where(std::string(it), placeholder);

        return query.to_sql();
    }

    template<Entity T, std::meta::info S>
    ers::Result<std::string> build_save() {
        ERS_QUICK_DBIO_USING;

        constexpr auto written = role_columns<T, S, Role::Out>();

        auto query = insert_into(std::string(table_name<T>()))
            | columns(written)
            | dummy_values(written.size())
            | on_conflict(conflict_target<T>());

        constexpr auto updates = upsert_updates<T, S>();

        if constexpr (updates.empty())
            query |= do_nothing();
        else
            query |= do_update(updates);

        if constexpr (!role_columns<T, S, Role::In>().empty())
            query |= returning(role_columns<T, S, Role::In>());

        return query.to_sql();
    }

    template<Entity T, std::meta::info S>
    ers::Result<std::string> build_update() {
        ERS_QUICK_DBIO_USING;

        auto query = update(std::string(table_name<T>()));

        for (const char* it : role_columns<T, S, Role::Out>())
            query |= assign(std::string(it), placeholder);

        for (const char* it : role_columns<T, S, Role::Key>())
            query |= where(std::string(it), placeholder);

        if constexpr (!role_columns<T, S, Role::In>().empty())
            query |= returning(role_columns<T, S, Role::In>());

        return query.to_sql();
    }

    template<Entity T, std::meta::info S>
    ers::Result<std::string> build_delete() {
        ERS_QUICK_DBIO_USING;

        auto query = delete_from(std::string(table_name<T>()));

        for (const char* it : role_columns<T, S, Role::Key>())
            query |= where(std::string(it), placeholder);

        if constexpr (!role_columns<T, S, Role::In>().empty())
            query |= returning(role_columns<T, S, Role::In>());

        return query.to_sql();
    }


    template<Entity T, std::meta::info S>
    ers::Result<std::string> build_sql() {
        constexpr auto type = std::meta::dealias(std::meta::type_of(S));

        if constexpr (type == ^^statement::Save)
            return build_save<T, S>();
        else if constexpr (type == ^^statement::Update)
            return build_update<T, S>();
        else if constexpr (type == ^^statement::Delete || (std::meta::has_template_arguments(type)
            && std::meta::template_of(type) == ^^statement::DeleteBy))
            return build_delete<T, S>();
        else
            return build_select<T, S>();
    }

    template<Entity T, std::meta::info S>
    const std::string& sql() {
        static const std::string cached = [] {
            auto result = build_sql<T, S>();
            return result ? std::move(*result) : std::string();
        }();

        return cached;
    }
}


// Parameter binding and row loading

namespace dbio::impl {
    // Out then Key, matching the emitted placeholder order.
    template<Entity T, std::meta::info S>
    void bind_params(const T& entity, pqxx::params& into) {
        constexpr auto out = role_columns<T, S, Role::Out>();
        constexpr auto key = role_columns<T, S, Role::Key>();

        const auto listed = [](std::span<const char* const> where, std::string_view what) {
            for (const char* it : where)
                if (std::string_view(it) == what)
                    return true;

            return false;
        };

        template for (constexpr auto m : std::define_static_array(entity_members<T>()))
            if (listed(out, std::meta::identifier_of(m)))
                into.append(entity.[:m:]);

        template for (constexpr auto m : std::define_static_array(entity_members<T>()))
            if (listed(key, std::meta::identifier_of(m)))
                into.append(entity.[:m:]);
    }

    template<Entity T, std::meta::info S>
    pqxx::params params_of(const T& entity) {
        pqxx::params out;
        bind_params<T, S>(entity, out);
        return out;
    }

    // In columns in declaration order, matching the emitted SELECT/RETURNING list.
    template<Entity T, std::meta::info S>
    bool load_row(T& entity, pqxx::row_ref row) {
        PqxxRowStream stream(row);

        constexpr auto in = role_columns<T, S, Role::In>();

        const auto listed = [](std::span<const char* const> where, std::string_view what) {
            for (const char* it : where)
                if (std::string_view(it) == what)
                    return true;

            return false;
        };

        template for (constexpr auto m : std::define_static_array(entity_members<T>()))
            if (listed(in, std::meta::identifier_of(m)))
                if (!stream.store(entity.[:m:]))
                    return false;

        return true;
    }
}


// Exports

namespace dbio::reflect {
    using impl::build_sql;
    using impl::sql;

    using impl::bind_params;
    using impl::params_of;
    using impl::load_row;
}
