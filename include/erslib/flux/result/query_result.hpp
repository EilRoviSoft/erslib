#pragma once

// std
#include <utility>

// pqxx
#include <pqxx/result>

// ers
#include <erslib/flux/result/collector.hpp>
#include <erslib/flux/result/row.hpp>
#include <erslib/flux/result/row_generator.hpp>


namespace flux::impl {
    class QueryResult {
    public:
        explicit QueryResult(pqxx::result content) :
            _content(std::move(content)) {
        }


        const pqxx::result& raw() const noexcept { return _content; }


        // T is a container (including ers::optional<Row>, via sql_collector<optional<T>>) ->
        // eager, 0..N (or 0..1) rows, never throws on row count.
        template<typename T>
            requires RowContainer<T>
        T get_as() const {
            return sql_collector<T>::collect(_content);
        }

        // T is itself the row type -> eager, exactly 1 row; throws otherwise.
        template<typename T>
            requires ReadableRow<T>
        T get_as() const {
            return read_one<T>(_content);
        }

        // Always lazy, always 0..N rows, never throws on row count.
        template<ReadableRow T>
        RowGenerator<T> view_as() const {
            return RowGenerator<T>(_content);
        }


    private:
        pqxx::result _content;
    };
}
