#pragma once

// std
#include <cstddef>
#include <ranges>
#include <utility>

// pqxx
#include <pqxx/result>

// ers
#include <erslib/flux/result/row.hpp>


namespace flux::impl {
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
                ++_inner_it;
                return *this;
            }
            Iterator operator++(int) {
                auto copy = *this;
                ++_inner_it;
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


        // Iterators

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
