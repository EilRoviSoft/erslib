#pragma once

// std
#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

// pqxx
#include <pqxx/result>
#include <pqxx/row>


// Concepts

namespace dbio::impl {
    template<typename T>
    concept ValidRow = std::constructible_from<T, pqxx::row_ref>;
}


// RowGenerator

namespace dbio::impl {
    template<typename T>
        requires ValidRow<T>
    class RowGenerator : public std::ranges::view_interface<RowGenerator<T>> {
    public:
        // Member types

        class Iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;

            Iterator() = default;
            explicit Iterator(pqxx::result::const_iterator it) :
                _inner_it(it) {
            }


            T operator*() const {
                return T(*_inner_it);
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
        };


    public:
        // Member functions

        RowGenerator() = default;
        explicit RowGenerator(pqxx::result content) :
            _content(std::move(content)) {
        }


        // Iterators for CRTP

        Iterator begin() const { return Iterator(_content.begin()); }
        Iterator end() const { return Iterator(_content.end()); }


        // Accessors

        [[nodiscard]]
        size_t size() const { return _content.size(); }


    private:
        pqxx::result _content;
    };
}


// Exports

namespace dbio {
    using impl::ValidRow;
    using impl::RowGenerator;
}
