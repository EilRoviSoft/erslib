#pragma once

// std
#include <memory>
#include <ranges>
#include <type_traits>

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
    template<typename T, typename R, typename... Ts>
    class TGroupIterator;


    template<typename T, typename... Ts>
    class TGroupView;
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


    template<typename... Ts>
    class Group final : public IGroup {
        template<typename, typename...>
        friend class TGroupView;


        static constexpr size_t size = sizeof...(Ts);
        static constexpr frozen::unordered_set<size_t, size> types = { ers::meta::type_hash_v<Ts>... };


    public:
        using storage_type = TrivialMap<std::tuple<Ts*...>>;


    public:
        // Constructors

        Group() = default;


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
            return m_storage.contains(id);
        }


        // Modifiers

        void add(Entity& entity) override {
            m_storage.emplace(
                entity.id(),
                std::make_tuple<Ts*...>(&entity.linked_components.at(ers::meta::type_hash_v<Ts>)->template get<Ts>()...)
            );
        }


        // Observers

        static size_t get_id() { return ers::hashing::combine<ers::DirectHash>(ers::meta::type_hash_v<Ts>...); }
        size_t id() const override { return get_id(); }


    protected:
        storage_type m_storage;
    };


    using GroupPtr = std::unique_ptr<IGroup>;


    template<typename... Ts>
    GroupPtr make_group() {
        return std::make_unique<Group<Ts...>>();
    }
}


// View

namespace ecs {
    template<typename T, typename... Ts>
    class TGroupView : public std::ranges::view_interface<TGroupView<T, Ts...>> {
        using iterator = T;
        friend iterator;


    public:
        TGroupView(Group<Ts...>& parent) :
            m_parent(parent) {
        }


        auto begin() const {
            return iterator(*this, m_parent.m_storage.begin());
        }
        auto end() const {
            return iterator(*this, m_parent.m_storage.end());
        }


        const Group<Ts...>::storage_type& base() const {
            return m_parent.m_storage;
        }


    protected:
        const Group<Ts...>& m_parent;
    };


    template<typename T, typename R, typename... Ts>
    class TGroupIterator {
    protected:
        using internal_iterator = Group<Ts...>::storage_type::const_iterator;


    public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;

        using parent_type = TGroupView<T, Ts...>;
        using value_type = std::conditional_t<
            std::is_void_v<R>,
            std::tuple<Ts&...>,
            std::tuple<R, Ts&...>
        >;
        using difference_type = std::ptrdiff_t;


    public:
        // Constructors

        TGroupIterator() = default;
        explicit TGroupIterator(const parent_type& parent, internal_iterator it) :
            m_parent(&parent),
            m_it(it) {
        }


        // Accessors

        value_type operator*() const {
            ERS_ASSERT(!m_parent || m_it == m_parent->base().end());
            return _interface()._value();
        }


        // Modifiers

        T& operator++() {
            ++m_it;
            return _interface();
        }
        T operator++(int) {
            auto temp = _interface();
            ++_interface();
            return temp;
        }


        // Comparing

        bool operator==(const TGroupIterator& other) const {
            return m_parent == other.m_parent && m_it == other.m_it;
        }


    protected:
        const parent_type* m_parent = nullptr;
        internal_iterator m_it;


    private:
        T& _interface() { return static_cast<T&>(*this); }
        const T& _interface() const { return static_cast<const T&>(*this); }
    };
}


// Iterators implementation

namespace ecs {
    template<typename... Ts>
    class GroupIterator : public TGroupIterator<GroupIterator<Ts...>, void, Ts...> {
        using base_type = TGroupIterator<GroupIterator, void, Ts...>;
        friend base_type;


    public:
        GroupIterator() = default;
        explicit GroupIterator(const base_type::parent_type& parent, base_type::internal_iterator it) :
            base_type(parent, it) {
        }


    private:
        base_type::value_type _value() const {
            return ers::util::pointers_to_references(this->m_it->second);
        }
    };


    template<typename... Ts>
    using GroupView = TGroupView<GroupIterator<Ts...>, Ts...>;
}


namespace ecs {
    template<typename... Ts>
    class GroupWithEntityIdIterator : public TGroupIterator<GroupWithEntityIdIterator<Ts...>, size_t, Ts...> {
        using base_type = TGroupIterator<GroupWithEntityIdIterator, size_t, Ts...>;
        friend base_type;


    public:
        GroupWithEntityIdIterator() = default;
        explicit GroupWithEntityIdIterator(const base_type::parent_type& parent, base_type::internal_iterator it) :
            base_type(parent, it) {
        }


    private:
        base_type::value_type _value() const {
            return ers::util::prepend(this->m_it->first, this->m_it->second);
        }
    };


    template<typename... Ts>
    using GroupWithEntityIdView = TGroupView<GroupWithEntityIdIterator<Ts...>, Ts...>;
}
