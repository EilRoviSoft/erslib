#pragma once

// std
#include <concepts>
#include <ranges>
#include <type_traits>
#include <utility>

// pqxx
#include <pqxx/pqxx>

// ers
#include <erslib/dbio/persistency.hpp>


namespace dbio {
    template<typename T>
    struct entity_traits;
}


// Concepts

namespace dbio::impl {
    template<typename T>
    concept ValidEntity =
        requires {
            typename entity_traits<T>::field_type;
            typename entity_traits<T>::layout_type;
            typename entity_traits<T>::config_type;
        }
        && std::is_enum_v<typename entity_traits<T>::field_type>
        && std::is_enum_v<typename entity_traits<T>::layout_type>
        && std::is_class_v<typename entity_traits<T>::config_type>
        && requires(typename entity_traits<T>::layout_type layout) {
            requires std::same_as<decltype(entity_traits<T>::template make_config<In>(layout)), typename entity_traits<T>::config_type>;
            requires std::same_as<decltype(entity_traits<T>::template make_config<Out>(layout)), typename entity_traits<T>::config_type>;
        };


    template<typename T>
    concept ValidRow = std::constructible_from<T, pqxx::row_ref>;
}


// EntityGenerator

namespace dbio::impl {
    template<typename T>
        requires ValidEntity<T>
    class EntityGenerator : public std::ranges::view_interface<EntityGenerator<T>> {
        using traits = entity_traits<T>;


    public:
        // Member types

        class Iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = T;
            using difference_type = std::ptrdiff_t;

            Iterator() = default;
            Iterator(pqxx::result::const_iterator it, traits::layout_type layout) :
                _inner_it(it),
                _layout(layout) {
            }


            T operator*() const {
                return T(*_inner_it, traits::template make_config<In>(_layout));
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
            traits::layout_type _layout {};
        };


    public:
        // Member functions

        EntityGenerator() = default;
        EntityGenerator(pqxx::result content, traits::layout_type layout) :
            _content(std::move(content)),
            _layout(layout) {
        }


        // Iterators for CRTP

        Iterator begin() const { return Iterator(_content.begin(), _layout); }
        Iterator end() const { return Iterator(_content.end(), _layout); }


        // Accessors

        [[nodiscard]]
        size_t size() const { return _content.size(); }


    private:
        pqxx::result _content;
        traits::layout_type _layout {};
    };
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
    using impl::ValidEntity;
    using impl::ValidRow;

    using impl::EntityGenerator;
    using impl::RowGenerator;
}
