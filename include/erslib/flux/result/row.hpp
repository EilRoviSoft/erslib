#pragma once

// std
#include <concepts>

// pqxx
#include <pqxx/result>
#include <pqxx/row>

// ers
#include <erslib/core/exception.hpp>
#include <erslib/core/type/optional.hpp>


namespace flux::impl {
    template<typename T>
    concept ValidRow = std::constructible_from<T, pqxx::row_ref>;


    template<typename T>
    struct row_reader;

    template<ValidRow T>
    struct row_reader<T> {
        struct state {};

        static state prepare(const pqxx::result&) { return {}; }
        static T read(pqxx::row_ref row, const state&) { return T(row); }
    };


    template<typename T>
    concept ReadableRow = requires(
        const pqxx::result& result,
        pqxx::row_ref row,
        const typename row_reader<T>::state& state
    ) {
        { row_reader<T>::prepare(result) } -> std::same_as<typename row_reader<T>::state>;
        { row_reader<T>::read(row, state) } -> std::same_as<T>;
    };


    template<ReadableRow Row, typename Fn>
    void for_each_row(const pqxx::result& from, Fn&& fn) {
        const auto state = row_reader<Row>::prepare(from);
        for (const auto& r : from)
            fn(row_reader<Row>::read(r, state));
    }


    template<ReadableRow T>
    T read_one(const pqxx::result& from) {
        if (from.size() != 1)
            throw ers::make_runtime_error("Expected exactly 1 row, got {}.", from.size());

        return row_reader<T>::read(from.one_row_ref(), row_reader<T>::prepare(from));
    }

    template<ReadableRow T>
    ers::optional<T> read_at_most_one(const pqxx::result& from) {
        if (from.empty())
            return ers::nullopt;

        if (from.size() > 1)
            throw ers::make_runtime_error("Expected at most 1 row, got {}.", from.size());

        return ers::optional<T>(row_reader<T>::read(from.one_row_ref(), row_reader<T>::prepare(from)));
    }
}
