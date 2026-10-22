#pragma once

// std
#include <utility>

// pqxx
#include <pqxx/dbtransaction>

// ers
#include <erslib/core/type/fixed_string.hpp>
#include <erslib/core/type/maybe.hpp>
#include <erslib/flux/query/clauses/returning.hpp>
#include <erslib/flux/reflect/crud/build.hpp>


// Implementation --------------------------------------------------------------

namespace flux::impl::crud {
    // The whole row back, so the identity and the defaults of the database are in it.
    template<reflect::Entity T>
    T insert(pqxx::dbtransaction& tx, const T& what) {
        return (insert(what) | clauses::returning_all()).exec(tx).template get_as<T>();
    }

    template<ers::fixed_string Constraint = "", reflect::Entity T>
    T upsert(pqxx::dbtransaction& tx, const T& what) {
        return (upsert<Constraint>(what) | clauses::returning_all()).exec(tx).template get_as<T>();
    }

    // 'As' is a projection, a struct with some of the columns of T.
    template<reflect::Entity T, typename As = T, typename... Keys>
    ers::maybe<As> find(pqxx::dbtransaction& tx, Keys&&... keys) {
        return (select<T, As>() | clauses::where(where_pk<T>(std::forward<Keys>(keys)...)))
            .exec(tx).template get_as<ers::maybe<As>>();
    }

    // With the soft-deleted rows.
    template<reflect::Entity T, typename As = T, typename... Keys>
    ers::maybe<As> find_all(pqxx::dbtransaction& tx, Keys&&... keys) {
        return (select_all<T, As>() | clauses::where(where_pk<T>(std::forward<Keys>(keys)...)))
            .exec(tx).template get_as<ers::maybe<As>>();
    }

    // Whether a row matched.
    template<reflect::Entity T>
    bool update(pqxx::dbtransaction& tx, const T& what) {
        return update(what).exec(tx).raw().affected_rows() > 0;
    }

    template<reflect::Entity T, typename Only>
    bool update(pqxx::dbtransaction& tx, const T& what, Only only) {
        return update(what, std::move(only)).exec(tx).raw().affected_rows() > 0;
    }

    template<reflect::Entity T, typename... Keys>
    bool erase(pqxx::dbtransaction& tx, Keys&&... keys) {
        return erase<T>(std::forward<Keys>(keys)...).exec(tx).raw().affected_rows() > 0;
    }

    template<reflect::Entity T, typename... Keys>
    bool erase_forever(pqxx::dbtransaction& tx, Keys&&... keys) {
        return erase_forever<T>(std::forward<Keys>(keys)...).exec(tx).raw().affected_rows() > 0;
    }

    template<reflect::Entity T, typename... Keys>
    bool restore(pqxx::dbtransaction& tx, Keys&&... keys) {
        return restore<T>(std::forward<Keys>(keys)...).exec(tx).raw().affected_rows() > 0;
    }
}
