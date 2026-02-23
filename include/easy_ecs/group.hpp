#pragma once

// std
#include <memory>
#include <ranges>

// frozen
#include <frozen/unordered_set.h>

// ers
#include <erslib/hashing/algorithm.hpp>
#include <erslib/meta/type_info.hpp>
#include <erslib/util/tuple.hpp>

// ecs
#include <easy_ecs/entity.hpp>
#include <easy_ecs/fwd.hpp>


// Forward declaration

namespace ecs {
    template<typename... Ts>
    class GroupIterator;
}


// Content

namespace ecs {
    class IGroup {
    public:
        virtual ~IGroup() = default;


        // Checkers

        virtual bool is_valid(const Entity& entity) const = 0;

        virtual bool has(size_t id) const = 0;


        // Modifiers

        virtual void add(Entity& entity) = 0;

        void add_if_valid(Entity& entity) {
            if (!has(entity.id()) && is_valid(entity))
                add(entity);
        }


        // Observers

        virtual size_t id() const = 0;
    };


    template<typename... Types>
    class TGroup final : public IGroup, std::ranges::view_interface<TGroup<Types...>> {
        friend GroupIterator<Types...>;

        static constexpr size_t size = sizeof...(Types);
        static constexpr frozen::unordered_set<size_t, size> types = { ers::meta::type_hash_v<Types>... };

    public:
        using container_type = TrivialMap<std::tuple<Types*...>>;
        using iterator = GroupIterator<Types...>;


        // Constructors

        TGroup() = default;


        // Checkers

        // We simply can count suitable components
        // because to satisfy group, entity should have all components.
        bool is_valid(const Entity& entity) const override {
            size_t count = 0;

            if (entity.linked_components.size() >= size) {
                for (const auto& component : entity.linked_components | std::views::values) {
                    if (types.contains(component->type()))
                        count++;
                }
            }

            return count == size;
        }

        bool has(size_t id) const override {
            return m_content.contains(id);
        }


        // Modifiers

        void add(Entity& entity) override {
            m_content.emplace(entity.id(), std::make_tuple<Types*...>(
                &entity.linked_components.at(ers::meta::type_hash_v<Types>)->template get<Types>()...));
        }


        // Observers

        static size_t get_id() { return ers::hashing::combine<ers::DirectHash>(ers::meta::type_hash_v<Types>...); }
        size_t id() const override { return get_id(); }


        iterator begin() const {
            return iterator(*this, m_content.begin());
        }
        iterator end() const {
            return iterator(*this, m_content.end());
        }

    protected:
        container_type m_content;
    };


    using GroupPtr = std::unique_ptr<IGroup>;


    template<typename... Ts>
    GroupPtr make_group() {
        return std::make_unique<TGroup<Ts...>>();
    }
}


// View

namespace ecs {
    template<typename... Ts>
    class GroupIterator {
    public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;

        using parent_type = TGroup<Ts...>;
        using difference_type = std::ptrdiff_t;

        using internal_iterator = parent_type::container_type::iterator;


        // Constructors

        GroupIterator() = default;
        explicit GroupIterator(const parent_type& parent, const internal_iterator& it) :
            m_parent(&parent),
            m_it(it) {
        }


        // Accessors

        std::tuple<Ts&...> operator*() const {
            ERS_ASSERT(!m_parent || m_it == m_parent->m_content.end());
            return ers::util::pointers_to_references(m_it->second);
        }


        GroupIterator& operator++() {
            ++m_it;
            return *this;
        }
        GroupIterator operator++(int) {
            auto temp = *this;
            ++*this;
            return temp;
        }


        // Comparing

        bool operator==(const GroupIterator& other) const {
            return m_it == other.m_it;
        }

    protected:
        const parent_type* m_parent = nullptr;
        internal_iterator m_it;
    };
}
