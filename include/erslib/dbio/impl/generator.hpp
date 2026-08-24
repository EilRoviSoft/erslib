#pragma once

// std
#include <concepts>
#include <ranges>
#include <utility>

// pqxx
#include <pqxx/result>
#include <pqxx/row>

// ers
#include <erslib/core/type/optional.hpp>
#include <erslib/core/type/result.hpp>


// Concepts

namespace dbio::impl {
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
    ers::Result<T> read_one(const pqxx::result& from) {
        if (from.size() != 1)
            return ers::make_error("Expected exactly 1 row, got {}.", from.size());

        return row_reader<T>::read(from.one_row_ref(), row_reader<T>::prepare(from));
    }

    template<ReadableRow T>
    ers::Result<ers::optional<T>> read_at_most_one(const pqxx::result& from) {
        if (from.empty())
            return ers::optional<T> {};

        if (from.size() > 1)
            return ers::make_error("Expected at most 1 row, got {}.", from.size());

        return ers::optional<T>(row_reader<T>::read(from.one_row_ref(), row_reader<T>::prepare(from)));
    }
}


// RowGenerator

namespace dbio::impl {
    template<ReadableRow T>
    class RowGenerator : public std::ranges::view_interface<RowGenerator<T>> {
        using reader = row_reader<T>;
        using state_t = typename reader::state;


    public:
        // Member types

        class Iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;

            Iterator() = default;
            Iterator(pqxx::result::const_iterator it, state_t state) :
                _inner_it(it),
                _state(std::move(state)) {
            }


            T operator*() const {
                return reader::read(*_inner_it, _state);
            }


            Iterator& operator++() {
                _inner_it++;
                return *this;
            }
            Iterator operator++(int) {
                auto copy = *this;
                _inner_it++;
                return copy;
            }


            bool operator==(const Iterator& other) const {
                return _inner_it == other._inner_it;
            }


        private:
            pqxx::result::const_iterator _inner_it;
            state_t _state {};
        };


    public:
        // Member functions

        RowGenerator() = default;
        explicit RowGenerator(pqxx::result content) :
            _content(std::move(content)),
            _state(reader::prepare(_content)) {
        }


        // Iterators for CRTP

        Iterator begin() const { return Iterator(_content.begin(), _state); }
        Iterator end() const { return Iterator(_content.end(), _state); }


        // Accessors

        [[nodiscard]]
        size_t size() const { return _content.size(); }


    private:
        pqxx::result _content;
        state_t _state {};
    };
}
