#pragma once

// std
#include <meta>
#include <string>
#include <utility>
#include <vector>

// ers
#include <erslib/dbio/clauses/assign.hpp>
#include <erslib/dbio/clauses/returning.hpp>
#include <erslib/dbio/clauses/target.hpp>
#include <erslib/dbio/clauses/values.hpp>
#include <erslib/dbio/clauses/where.hpp>
#include <erslib/dbio/reflect/schema.hpp>
#include <erslib/dbio/reflect/value.hpp>
#include <erslib/dbio/sql/policy.hpp>


namespace dbio::impl::sql {
    template<Entity T, typename Kind, typename Role>
    std::vector<std::string> column_names() {
        std::vector<std::string> out;

        template for (constexpr auto m : std::define_static_array(columns<T>()))
            if constexpr (column_policy<T, Kind, Role>::template includes<m>())
                out.emplace_back(column_name<T, m>());

        return out;
    }

    template<Entity T, typename Kind, typename Role>
    std::vector<Clause> column_list() {
        return clauses::columns(column_names<T, Kind, Role>());
    }

    template<Entity T, typename Kind, typename Role>
    std::vector<Clause> returning_list() {
        return clauses::returning(column_names<T, Kind, Role>());
    }


    template<Entity T, typename Kind, typename Role>
    Clause values_of(const T& what) {
        std::vector<binder_t> binders;

        template for (constexpr auto m : std::define_static_array(columns<T>()))
            if constexpr (column_policy<T, Kind, Role>::template includes<m>())
                binders.emplace_back(value_binder(what.[:m:]));

        return make_clause<ValuesClause>(std::move(binders));
    }

    template<Entity T, typename Kind, typename Role>
    std::vector<Clause> assignments_of(const T& what) {
        std::vector<Clause> out;

        template for (constexpr auto m : std::define_static_array(columns<T>()))
            if constexpr (column_policy<T, Kind, Role>::template includes<m>())
                out.emplace_back(make_clause<AssignClause>(
                    std::string(column_name<T, m>()), value_binder(what.[:m:])));

        return out;
    }

    template<Entity T, typename Kind, typename Role>
    std::vector<Clause> conditions_of(const T& what) {
        std::vector<Clause> out;

        template for (constexpr auto m : std::define_static_array(columns<T>()))
            if constexpr (column_policy<T, Kind, Role>::template includes<m>())
                out.emplace_back(make_clause<WhereOpClause>(
                    std::string(column_name<T, m>()), Op::Eq, value_binder(what.[:m:])));

        return out;
    }


    template<Entity T, typename... Keys>
    std::vector<Clause> conditions_for(Keys&&... keys) {
        constexpr auto names = primary_key<T>();

        static_assert(sizeof...(Keys) == names.size(),
            "wrong number of key values for this entity's primary key");

        std::vector<Clause> out;
        out.reserve(sizeof...(Keys));

        size_t at = 0;
        (out.emplace_back(make_clause<WhereOpClause>(
            std::string(names[at++]), Op::Eq, value_binder(std::forward<Keys>(keys)))), ...);

        return out;
    }
}
