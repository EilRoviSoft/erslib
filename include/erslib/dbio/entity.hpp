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


// Traits

namespace dbio {
    // Specialize for every generated entity.
    // The generated header provides a specialization exposing field/layout/config types and a make_config().
    template<typename T>
    struct entity_traits;

    template<typename T>
    concept ValidEntityTraits =
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
}


// EntityGenerator

namespace dbio {
    // Owning lazy view over a pqxx::result that materializes one entity per row.
    // Iterators do not support operator->, and operator* produces a
    // freshly constructed entity from the underlying pqxx::row (i.e. a copy).
    template<typename T>
        requires ValidEntityTraits<T>
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
                m_it(it),
                m_layout(layout) {
            }

            
            T operator*() const {
                return T(*m_it, traits::template make_config<In>(m_layout));
            }

            
            Iterator& operator++() {
                m_it++;
                return *this;
            }
            Iterator operator++(int) {
                auto copy = *this;
                m_it++;
                return copy;
            }


            bool operator==(const Iterator& other) const {
                return m_it == other.m_it;
            }


        private:
            pqxx::result::const_iterator m_it;
            traits::layout_type m_layout {};
        };


    public:
        // Member functions

        EntityGenerator() = default;
        EntityGenerator(pqxx::result content, traits::layout_type layout) :
            m_content(std::move(content)),
            m_layout(layout) {
        }


        // Iterators for CRTP

        Iterator begin() const { return Iterator(m_content.begin(), m_layout); }
        Iterator end() const { return Iterator(m_content.end(), m_layout); }


        // Accessors

        [[nodiscard]]
        size_t size() const { return m_content.size(); }


    private:
        pqxx::result m_content;
        traits::layout_type m_layout {};
    };
}


// RowGenerator

namespace dbio {
    template<typename T>
    concept RowConstructible = std::constructible_from<T, const pqxx::row&>;

    // Owning lazy view over a pqxx::result that materializes one lightweight row struct per row.
    // Unlike EntityGenerator it needs no entity_traits:
    // T only has to be constructible from a pqxx::row (used by generated custom-query results).
    // Iterators do not support operator->, and operator* produces a freshly constructed T
    // from the underlying pqxx::row (i.e. a copy).
    template<typename T>
        requires RowConstructible<T>
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
                m_it(it) {
            }


            T operator*() const {
                return T(*m_it);
            }


            Iterator& operator++() {
                m_it++;
                return *this;
            }
            Iterator operator++(int) {
                auto copy = *this;
                m_it++;
                return copy;
            }


            bool operator==(const Iterator& other) const {
                return m_it == other.m_it;
            }


        private:
            pqxx::result::const_iterator m_it;
        };


    public:
        // Member functions

        RowGenerator() = default;
        explicit RowGenerator(pqxx::result content) :
            m_content(std::move(content)) {
        }


        // Iterators for CRTP

        Iterator begin() const { return Iterator(m_content.begin()); }
        Iterator end() const { return Iterator(m_content.end()); }


        // Accessors

        [[nodiscard]]
        size_t size() const { return m_content.size(); }


    private:
        pqxx::result m_content;
    };
}
