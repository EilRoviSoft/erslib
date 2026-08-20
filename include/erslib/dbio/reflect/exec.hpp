#pragma once

// std
#include <utility>
#include <vector>

// pqxx
#include <pqxx/dbtransaction>

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/result.hpp>
#include <erslib/dbio/impl/query.hpp>
#include <erslib/dbio/reflect/row.hpp>
#include <erslib/dbio/sql/insert.hpp>
#include <erslib/dbio/sql/remove.hpp>
#include <erslib/dbio/sql/save.hpp>
#include <erslib/dbio/sql/select.hpp>
#include <erslib/dbio/sql/update.hpp>


// Running a query as entities

namespace dbio::impl {
    template<Entity T>
    ers::Result<std::vector<T>> exec_as(pqxx::dbtransaction& tx, const Query& what) {
        auto r = what.exec(tx);
        if (!r)
            return r.error();

        const ColumnIndex<T> index = map_columns<T>(*r);

        std::vector<T> out;
        out.reserve(r->size());

        for (const auto row : *r)
            out.emplace_back(from_row<T>(row, index));

        return out;
    }

    template<Entity T>
    ers::Result<ers::optional<T>> exec_one(pqxx::dbtransaction& tx, const Query& what) {
        auto r = what.exec(tx);
        if (!r)
            return r.error();

        if (r->empty())
            return ers::optional<T>(ers::nullopt);

        const ColumnIndex<T> index = map_columns<T>(*r);

        return ers::optional<T>(from_row<T>(*r->begin(), index));
    }
}


// Entity operations

namespace dbio::impl {
    template<Entity T>
    ers::Status write_identity(T& into, const pqxx::result& from) {
        if constexpr (has_identity<T>()) {
            if (from.empty())
                return ers::make_error("Expected a returned row for '{}'.", table_name<T>());

            read_row(into, *from.begin(), map_columns<T>(from));
        }

        return ers::ok;
    }


    template<Entity T>
    ers::Status save(pqxx::dbtransaction& tx, T& what) {
        auto r = sql::save<T>(what).exec(tx);
        if (!r)
            return r.error();

        return write_identity(what, *r);
    }

    template<Entity T>
    ers::Status insert(pqxx::dbtransaction& tx, T& what) {
        auto r = sql::insert<T>(what).exec(tx);
        if (!r)
            return r.error();

        return write_identity(what, *r);
    }

    template<Entity T>
    ers::Status update(pqxx::dbtransaction& tx, const T& what) {
        return sql::update<T>(what).exec_and_discard(tx);
    }

    template<Entity T>
    ers::Status remove(pqxx::dbtransaction& tx, const T& what) {
        return sql::remove<T>(what).exec_and_discard(tx);
    }


    template<Entity T, typename... Keys>
    ers::Result<ers::optional<T>> load_by_pk(pqxx::dbtransaction& tx, Keys&&... keys) {
        return exec_one<T>(tx, sql::load_by_pk<T>(std::forward<Keys>(keys)...));
    }

    template<Entity T>
    ers::Result<std::vector<T>> load_all(pqxx::dbtransaction& tx) {
        return exec_as<T>(tx, sql::select_all<T>());
    }
}
