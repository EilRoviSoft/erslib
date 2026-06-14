#pragma once

// std
#include <memory>
#include <ranges>

// frozen
#include <frozen/unordered_set.h>

// ers
#include <erslib/macro.hpp>
#include <erslib/hashing/algorithm.hpp>
#include <erslib/util/tuple.hpp>

// aengine
#include <aengine/fwd.hpp>

// ecs
#include <easy_ecs/component.hpp>
#include <easy_ecs/entity.hpp>


// Forward declaration

namespace ecs {
    template<typename Iterator, typename... Tags>
    class TGroupView;

    template<typename... Tags>
    class GroupIterator;

    template<typename... Tags>
    class GroupWithEntityIdIterator;
}


// Internal

// It's needed to avoid errors with calling forward-declared types.

namespace ecs::internal {
    [[nodiscard]]
    bool is_valid(const Registry& registry, size_t eid, size_t cid);

    [[nodiscard]]
    void* get_component(const Registry& registry, size_t eid, size_t cid);
}


// IGroup

namespace ecs {
    class IGroup {
    public:
        // Member functions

        virtual ~IGroup() = default;


        // Checkers

        [[nodiscard]]
        virtual bool is_valid(const Registry& registry, const IEntity& entity) const = 0;
        [[nodiscard]]
        virtual bool has(size_t eid) const = 0;


        // Modifiers

        virtual void add(const Registry& registry, IEntity& entity) = 0;

        void try_add(const Registry& registry, IEntity& entity);

        [[nodiscard]] virtual size_t id() const = 0;
    };
}


// TGroup

namespace ecs {
    template<ComponentTag... Tags>
    class TGroup final : public IGroup {
        template<typename, typename...>
        friend class TGroupView;


    public:
        using storage_type = aengine::TrivialMap<
            std::tuple<component_value_t<Tags>*...>
        >;


        // IGroup interface

        [[nodiscard]]
        bool is_valid(const Registry& registry, const IEntity& entity) const override {
            return (internal::is_valid(registry, entity.id(), component_id<Tags>()) && ...);
        }

        [[nodiscard]]
        bool has(size_t eid) const override {
            return m_storage.contains(eid);
        }

        void add(const Registry& registry, IEntity& entity) override {
            auto components = std::make_tuple(static_cast<component_value_t<Tags>* ERS_RESTRICT>(
                internal::get_component(registry, entity.id(), component_id<Tags>()))...);

            m_storage.emplace(entity.id(), std::move(components));
        }

        [[nodiscard]]
        static size_t get_id() {
            return ers::hashing::combine<ers::RapidHash>(component_id<Tags>()...);
        }

        [[nodiscard]]
        size_t id() const override { return get_id(); }


    protected:
        storage_type m_storage;
    };


    using GroupPtr = std::unique_ptr<IGroup>;

    template<ComponentTag... Tags>
    GroupPtr make_group() { return std::make_unique<TGroup<Tags...>>(); }
}


// View

namespace ecs {
    template<typename T, typename... Ts>
    class TGroupView : public std::ranges::view_interface<TGroupView<T, Ts...>> {
        using iterator = T;
        friend iterator;


    public:
        TGroupView(TGroup<Ts...>& parent) :
            m_parent(parent) {
        }


        auto begin() const { return iterator(*this, m_parent.m_storage.begin()); }
        auto end() const { return iterator(*this, m_parent.m_storage.end()); }


        const TGroup<Ts...>::storage_type& base() const { return m_parent.m_storage; }


    protected:
        const TGroup<Ts...>& m_parent;
    };
}


// Iterators

namespace ecs {
    template<typename T, typename R, typename... Ts>
    class TGroupIterator {
    protected:
        using internal_iterator = TGroup<Ts...>::storage_type::const_iterator;


    public:
        using iterator_concept = std::forward_iterator_tag;
        using iterator_category = std::forward_iterator_tag;

        using parent_type = TGroupView<T, Ts...>;
        using value_type = std::conditional_t<
            std::is_void_v<R>,
            std::tuple<typename Ts::value_type&...>,
            std::tuple<R, typename Ts::value_type&...>
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
            auto&& [...args] = this->m_it->second;
            return { this->m_it->first, *args... };
        }
    };


    template<typename... Ts>
    using GroupWithEntityIdView = TGroupView<GroupWithEntityIdIterator<Ts...>, Ts...>;
}
