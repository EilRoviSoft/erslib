#pragma once

// std
#include <meta>
#include <vector>

// ers
#include <erslib/dbio/eval.hpp>
#include <erslib/dbio/reflect/query.hpp>


// Specs

namespace dbio::impl {
    // Columns only; metadata members never become storage.
    template<typename T>
    consteval std::vector<std::meta::info> entity_specs() {
        std::vector<std::meta::info> out;

        template for (constexpr auto m : std::define_static_array(columns<T>()))
            out.emplace_back(std::meta::data_member_spec(
                std::meta::type_of(m),
                { .name = column_name<m>() }));

        return out;
    }
}


// Callables

namespace dbio::impl {
    template<typename T, std::meta::info S>
    struct select_all_fn {
        ers::Result<std::vector<T>> operator()(pqxx::dbtransaction& tx) const ERS_DBIO_TRY_EVAL {
            const pqxx::result content = tx.exec(sql<T, S>());

            std::vector<T> out;
            out.reserve(content.size());

            for (const auto row : content) {
                T entity {};
                if (!load_row<T, S>(entity, row))
                    return ers::make_error("Failed to read a row of '{}'.", table_name<T>());

                out.emplace_back(std::move(entity));
            }

            return out;
        }
        ERS_DBIO_CATCH_EVAL_ERRORS
    };

    template<typename T, std::meta::info S>
    struct write_fn {
        ers::Status operator()(pqxx::dbtransaction& tx, T& entity) const ERS_DBIO_TRY_EVAL {
            const pqxx::result content = tx.exec(sql<T, S>(), params_of<T, S>(entity));

            if constexpr (!role_columns<T, S, Role::In>().empty())
                if (!content.empty() && !load_row<T, S>(entity, content.one_row_ref()))
                    return ers::make_error("Failed to read the result of '{}'.", table_name<T>());

            return ers::ok;
        }
        ERS_DBIO_CATCH_EVAL_ERRORS
    };

    template<typename T, std::meta::info S>
    struct load_fn {
        ers::Status operator()(pqxx::dbtransaction& tx, T& entity) const ERS_DBIO_TRY_EVAL {
            const pqxx::result content = tx.exec(sql<T, S>(), params_of<T, S>(entity));

            if (content.empty())
                return ers::make_error("No row of '{}' matched.", table_name<T>());

            if (!load_row<T, S>(entity, content.one_row_ref()))
                return ers::make_error("Failed to read a row of '{}'.", table_name<T>());

            return ers::ok;
        }
        ERS_DBIO_CATCH_EVAL_ERRORS
    };


    consteval std::meta::info callable_for(std::meta::info entity, std::meta::info s) {
        const std::meta::info type = std::meta::dealias(std::meta::type_of(s));
        const std::meta::info self = std::meta::reflect_constant(s);

        if (type == ^^statement::SelectAll)
            return std::meta::substitute(^^select_all_fn, { entity, self });

        if (type == ^^statement::Save || type == ^^statement::Update
            || type == ^^statement::Delete)
            return std::meta::substitute(^^write_fn, { entity, self });

        if (std::meta::has_template_arguments(type)
            && std::meta::template_of(type) == ^^statement::DeleteBy)
            return std::meta::substitute(^^write_fn, { entity, self });

        return std::meta::substitute(^^load_fn, { entity, self });
    }

    // One callable member per statement; the member name is the method name.
    template<typename T>
    consteval std::vector<std::meta::info> api_specs() {
        std::vector<std::meta::info> out;

        for (const auto m : statements<T>())
            out.emplace_back(std::meta::data_member_spec(
                callable_for(^^T, m),
                { .name = std::meta::identifier_of(m) }));

        return out;
    }
}


// Exports

namespace dbio::reflect {
    using impl::entity_specs;
    using impl::api_specs;
}


// Declares the entity's members and its api object from Definition<Entity>.
#define ERS_DBIO_ENTITY(Entity, Api, Object)                                                                           \
    consteval {                                                                                                        \
        std::meta::define_aggregate(^^Entity, dbio::reflect::entity_specs<Entity>());                                  \
    }                                                                                                                  \
    struct Api;                                                                                                        \
    consteval {                                                                                                        \
        std::meta::define_aggregate(^^Api, dbio::reflect::api_specs<Entity>());                                        \
    }                                                                                                                  \
    inline constexpr Api Object {}
