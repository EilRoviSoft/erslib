#pragma once

// std
#include <cstddef>
#include <memory>
#include <memory_resource>

// ers
#include <erslib/assert.hpp>
#include <erslib/concept/sbo.hpp>
#include <erslib/meta/type_info.hpp>


// Forward declaration

namespace ers::internal {
    template<size_t Size, size_t Align>
    class TAny;
}


// Details

namespace ers::internal {
    template<size_t Size, size_t Align>
    struct TAnyStorage {
        union {
            void* heap;
            alignas(Align) std::byte buffer[Size];
        };
    };

    // "is_sbo_applicable" checks 0 size already.
    template<size_t Align>
    struct TAnyStorage<0, Align> {
        void* heap;
    };

    template<size_t Size, size_t Align>
    struct TAnyVtable {
        using target_type = TAny<Size, Align>;

        void (*destroy)(target_type& obj);
        void (*dealloc)(target_type& obj);
        void (*copy)(target_type& dst, const void* src);
        void (*move)(target_type& dst, void* src);
        void (*change_resource)(target_type& obj, std::pmr::memory_resource* mr);

        template<typename T>
        static T* impl_alloc(target_type& obj);

        template<typename T>
        static void impl_destroy(target_type& obj);

        template<typename T>
        static void impl_dealloc(target_type& obj);

        template<typename T>
        static void impl_copy(target_type& dst, const void* src);

        template<typename T>
        static void impl_move(target_type& dst, void* src);

        template<typename T>
        static void impl_change_resource(target_type& obj, std::pmr::memory_resource* mr);

        template<typename T>
        static constexpr auto make() {
            TAnyVtable result;

            result.destroy = &impl_destroy<T>;
            result.dealloc = &impl_dealloc<T>;

            result.copy = &impl_copy<T>;
            result.move = &impl_move<T>;

            result.change_resource = &impl_change_resource<T>;

            return result;
        }

        template<typename T>
        static constexpr const auto& get() {
            static constexpr auto instance = make<T>();
            return instance;
        }
    };
}


// Implementation

namespace ers::internal {
    template<size_t Size, size_t Align>
    class TAny {
        using storage_type = TAnyStorage<Size, Align>;
        using vtable_type = TAnyVtable<Size, Align>;

        friend vtable_type;

    public:
        struct placeholder_t {};


        // Default Constructor

        TAny() :
            m_mr(nullptr),
            m_vtable(nullptr),
            m_type(meta::type_hash_v<placeholder_t>),
            m_policy(SboPolicy::Empty),
            m_storage { .heap = nullptr } {
        }


        // Copy

        TAny(const TAny& other) :
            m_mr(other.m_mr),
            m_vtable(other.m_vtable),
            m_type(meta::type_hash_v<placeholder_t>),
            m_policy(SboPolicy::Empty),
            m_storage { .heap = nullptr } {
            m_vtable->copy(*this, other._data());
        }
        TAny& operator=(const TAny& other) {
            _soft_reset();

            m_mr = other.m_mr;
            m_vtable = other.m_vtable;
            m_type = other.m_type;
            m_vtable->copy(*this, other._data());

            return *this;
        }


        // Move

        TAny(TAny&& other) noexcept :
            m_mr(other.m_mr),
            m_vtable(other.m_vtable),
            m_type(meta::type_hash_v<placeholder_t>),
            m_policy(SboPolicy::Empty),
            m_storage { .heap = nullptr } {
            m_vtable->move(*this, other._data());
        }
        TAny& operator=(TAny&& other) noexcept {
            _soft_reset();

            m_mr = other.m_mr;
            m_vtable = other.m_vtable;
            m_type = other.m_type;
            m_vtable->move(*this, other._data());

            return *this;
        }


        // Value initializer

        template<typename T>
            requires (!std::is_same_v<T, TAny>)
        TAny(T&& v, std::pmr::memory_resource* provided_mr = std::pmr::get_default_resource()) :
            m_mr(nullptr),
            m_vtable(nullptr),
            m_type(meta::type_hash_v<placeholder_t>),
            m_policy(SboPolicy::Empty),
            m_storage { .heap = nullptr } {
            emplace_with_resource<T>(provided_mr, std::forward<T>(v));
        }
        template<typename T>
            requires (!std::is_same_v<T, TAny>)
        TAny& operator=(T&& v) {
            m_policy = SboPolicy::Empty;

            emplace<T>(std::forward<T>(v));

            return *this;
        }


        // Value in-place initializer

        template<typename T, typename... Args>
        explicit TAny(std::in_place_type_t<T>, Args&&... args) :
            m_mr(nullptr),
            m_vtable(nullptr),
            m_type(meta::type_hash_v<placeholder_t>),
            m_policy(SboPolicy::Empty),
            m_storage { .heap = nullptr } {
            emplace<T>(std::forward<Args>(args)...);
        }
        template<typename T, typename... Args>
        explicit TAny(std::in_place_type_t<T>, std::pmr::memory_resource* provided_mr, Args&&... args) :
            m_mr(nullptr),
            m_vtable(nullptr),
            m_type(meta::type_hash_v<placeholder_t>),
            m_policy(SboPolicy::Empty),
            m_storage { .heap = nullptr } {
            emplace_with_resource<T>(provided_mr, std::forward<Args>(args)...);
        }


        // Destructor

        ~TAny() {
            _soft_reset();
        }


        // Modifiers

        void change_resource(std::pmr::memory_resource* provided_mr) {
            m_vtable->change_resource(*this, provided_mr);
        }

        void reset() {
            if (m_policy != SboPolicy::Empty) {
                _soft_reset();

                m_mr = nullptr;
                m_vtable = nullptr;
                m_type = meta::type_hash_v<placeholder_t>;
                m_policy = SboPolicy::Empty;
            }
        }

        template<typename T, typename... Args>
            requires std::is_constructible_v<std::decay_t<T>, Args...>
            && std::is_copy_constructible_v<std::decay_t<T>>
        auto emplace(Args&&... args) {
            return emplace_with_resource<T>(
                m_mr ? m_mr : std::pmr::get_default_resource(),
                std::forward<Args>(args)...
            );
        }

        template<typename T, typename... Args>
            requires std::is_constructible_v<std::decay_t<T>, Args...>
            && std::is_copy_constructible_v<std::decay_t<T>>
        auto emplace_with_resource(std::pmr::memory_resource* provided_mr, Args&&... args) {
            using U = std::decay_t<T>;

            U* ptr;
            if (m_policy != SboPolicy::Empty && same_as<U>() && *m_mr == *provided_mr) {
                ptr = _data_as<U>();

                m_vtable->destroy(*this);
                std::construct_at(ptr, std::forward<Args>(args)...);
            } else {
                if (m_policy != SboPolicy::Empty)
                    _soft_reset();

                m_mr = provided_mr;
                m_vtable = &vtable_type::template get<U>();
                m_type = meta::type_hash_v<U>;

                ptr = vtable_type::template impl_alloc<U>(*this);
                std::construct_at(ptr, std::forward<Args>(args)...);
            }

            return ptr;
        }


        // Observers

        [[nodiscard]]
        constexpr bool has_value() const { return m_policy == SboPolicy::Empty; }

        template<typename T>
        [[nodiscard]]
        constexpr bool same_as() const { return m_type == meta::type_hash_v<T>; }

        [[nodiscard]]
        constexpr size_t type() const { return m_type; }


        // Accessors

        template<typename T>
        [[nodiscard]]
        T& get() { return *_data_as<T>(); }
        template<typename T>
        [[nodiscard]]
        const T& get() const { return *_data_as<T>(); }

    protected:
        std::pmr::memory_resource* m_mr;
        const vtable_type* m_vtable;
        size_t m_type;
        SboPolicy m_policy;
        storage_type m_storage;

    private:
        // Modifiers

        // Can only be called
        // if it's guaranteed that "mr", "vtable" and "type" will be replaced after that.
        void _soft_reset() {
            m_vtable->destroy(*this);
            m_vtable->dealloc(*this);
        }

        template<typename T>
        void _prepare_to_assign() {
            if (m_type == meta::type_hash_v<T>)
                return;

            if (m_policy != SboPolicy::Empty)
                m_vtable->destroy(*this);

            m_vtable->dealloc(*this);

            vtable_type::template impl_alloc<T>(*this);
            m_type = ers::meta::type_hash_v<T>;
        }


        // Accessors

        [[nodiscard]]
        void* _data() {
            switch (m_policy) {
            case SboPolicy::Dynamic:
                return m_storage.heap;

            case SboPolicy::Embedded:
                return m_storage.buffer;

            default:
                return nullptr;
            }
        }
        [[nodiscard]]
        const void* _data() const {
            switch (m_policy) {
            case SboPolicy::Dynamic:
                return m_storage.heap;

            case SboPolicy::Embedded:
                return m_storage.buffer;

            default:
                return nullptr;
            }
        }

        template<typename T>
        [[nodiscard]]
        T* _data_as() {
            if constexpr (is_sbo_applicable_v<T, Size, Align>)
                return reinterpret_cast<T*>(m_storage.buffer);
            else
                return static_cast<T*>(m_storage.heap);
        }
        template<typename T>
        [[nodiscard]]
        const T* _data_as() const {
            if constexpr (is_sbo_applicable_v<T, Size, Align>)
                return reinterpret_cast<const T*>(m_storage.buffer);
            else
                return static_cast<const T*>(m_storage.heap);
        }
    };
}


// Implementation

namespace ers::internal {
    template<size_t Size, size_t Align>
    template<typename T>
    T* TAnyVtable<Size, Align>::impl_alloc(target_type& obj) {
        T* result;

        if constexpr (!is_sbo_applicable_v<T, Size, Align>) {
            obj.m_storage.heap = obj.m_mr->allocate(sizeof(T), alignof(T));
            obj.m_policy = SboPolicy::Dynamic;
            result = static_cast<T*>(obj.m_storage.heap);
        } else {
            obj.m_policy = SboPolicy::Embedded;
            result = reinterpret_cast<T*>(obj.m_storage.buffer);
        }

        return result;
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_destroy(target_type& obj) {
        if constexpr (std::is_trivially_destructible_v<T>)
            return;

        if constexpr (is_sbo_applicable_v<T, Size, Align>) {
            ERS_ASSERT(obj.m_policy == SboPolicy::Embedded);
            std::destroy_at(reinterpret_cast<T*>(obj.m_storage.buffer));
        } else {
            ERS_ASSERT(obj.m_policy == SboPolicy::Dynamic);
            std::destroy_at(static_cast<T*>(obj.m_storage.heap));
        }
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_dealloc(target_type& obj) {
        if constexpr (!is_sbo_applicable_v<T, Size, Align>)
            obj.m_mr->deallocate(obj.m_storage.heap, sizeof(T), alignof(T));
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_copy(target_type& dst, const void* src) {
        dst.template _prepare_to_assign<T>();

        if constexpr (std::is_trivially_copyable_v<T>)
            std::memcpy(dst.template _data_as<T>(), static_cast<const T*>(src), sizeof(T));
        else
            std::construct_at(dst.template _data_as<T>(), *static_cast<const T*>(src));
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_move(target_type& dst, void* src) {
        dst.template _prepare_to_assign<T>();

        if constexpr (std::is_trivially_move_constructible_v<T> && std::is_trivially_destructible_v<T>)
            std::memcpy(dst.template _data_as<T>(), static_cast<const T*>(src), sizeof(T));
        else
            std::construct_at(dst.template _data_as<T>(), std::move(*static_cast<T*>(src)));
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_change_resource(target_type& obj, std::pmr::memory_resource* mr) {
        if (*mr == *obj.m_mr)
            return;

        // We should move heap's storage in case it is allocated.
        // But otherwise we can skip it entirely.

        if constexpr (!is_sbo_applicable_v<T, Size, Align>) {
            auto old_mr = obj.m_mr;

            void* old_heap = obj.m_storage.heap;
            void* new_heap = obj.m_mr->allocate(sizeof(T), alignof(T));

            if constexpr (std::is_trivially_copyable_v<T>)
                std::memcpy(new_heap, old_heap, sizeof(T));
            else
                std::construct_at(static_cast<T*>(new_heap), std::move(*static_cast<T*>(old_heap)));

            if constexpr (!std::is_trivially_destructible_v<T>)
                std::destroy_at(static_cast<T*>(old_heap));

            old_mr->deallocate(old_heap, sizeof(T), alignof(T));

            obj.m_storage.heap = new_heap;
        }

        obj.m_mr = mr;
    }
}


// General use-case

namespace ers::pmr {
    using Any = internal::TAny<32, alignof(std::max_align_t)>;
}
