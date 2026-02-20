#pragma once

// std
#include <cstddef>
#include <memory_resource>

// ers
#include <erslib/assert.hpp>
#include <erslib/concept/sbo.hpp>
#include <erslib/meta/type_info.hpp>


// Forward declaration

namespace ers::internal {
    template<size_t Size, size_t Align>
    struct TAnyImpl;
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
        using target_type = TAnyImpl<Size, Align>;

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
        static void impl_trivial_copy(target_type& dst, const void* src);

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

            if constexpr (std::is_trivially_copyable_v<T>) {
                result.copy = &impl_trivial_copy<T>;
                result.move = &impl_trivial_copy<T>;
            } else {
                result.copy = &impl_copy<T>;
                result.move = &impl_move<T>;
            }

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


// Declaration

namespace ers::internal {
    template<size_t Size, size_t Align>
    struct TAnyImpl {
        // Aliases

        using storage_type = TAnyStorage<Size, Align>;
        using vtable_type = TAnyVtable<Size, Align>;

        // Members

        std::pmr::memory_resource* mr;
        const vtable_type* vtable;
        size_t type;
        SboPolicy policy;
        storage_type storage;

        // Methods

        // Returns if types are the same.
        // Calls "destroy" if is not empty.
        template<typename T>
        void prepare_to_assign() {
            if (type == meta::type_hash_v<T>)
                return;

            if (policy != SboPolicy::Empty)
                vtable->destroy(*this);

            vtable->dealloc(*this);
            vtable_type::template impl_alloc<T>(*this);
        }

        // Setters

        // Accessors

        [[nodiscard]]
        void* data() {
            switch (policy) {
                case SboPolicy::Dynamic:
                    return storage.heap;

                case SboPolicy::Embedded:
                    return storage.buffer;

                default:
                    return nullptr;
            }
        }
        [[nodiscard]]
        const void* data() const {
            switch (policy) {
                case SboPolicy::Dynamic:
                    return storage.heap;

                case SboPolicy::Embedded:
                    return storage.buffer;

                default:
                    return nullptr;
            }
        }

        template<typename T>
        T* data_as() {
            if constexpr (is_sbo_applicable_v<T, Size, Align>)
                return reinterpret_cast<T*>(storage.buffer);
            else
                return static_cast<T*>(storage.heap);
        }
        template<typename T>
        const T* data_as() const {
            if constexpr (is_sbo_applicable_v<T, Size, Align>)
                return reinterpret_cast<const T*>(storage.buffer);
            else
                return static_cast<const T*>(storage.heap);
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
            obj.storage.heap = obj.mr->allocate(sizeof(T), alignof(T));
            obj.policy = SboPolicy::Dynamic;
            result = static_cast<T*>(obj.storage.heap);
        } else {
            obj.policy = SboPolicy::Embedded;
            result = static_cast<T*>(obj.storage.buffer);
        }

        return result;
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_destroy(target_type& obj) {
        if constexpr (std::is_trivially_destructible_v<T>)
            return;

        if constexpr (is_sbo_applicable_v<T, Size, Align>) {
            ERS_ASSERT(obj.policy == SboPolicy::Embedded);
            std::destroy_at(reinterpret_cast<T*>(obj.storage.buffer));
        } else {
            ERS_ASSERT(obj.policy == SboPolicy::Dynamic);
            std::destroy_at(static_cast<T*>(obj.storage.heap));
        }
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_dealloc(target_type& obj) {
        if constexpr (!is_sbo_applicable_v<T, Size, Align>)
            obj.mr->deallocate(obj.storage.heap, sizeof(T), alignof(T));
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_trivial_copy(target_type& dst, const void* src) {
        dst.template prepare_to_assign<T>();
        std::memcpy(dst.template data_as<T>(), static_cast<const T*>(src), sizeof(T));
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_copy(target_type& dst, const void* src) {
        dst.template prepare_to_assign<T>();
        std::construct_at(dst.template data_as<T>(), *static_cast<const T*>(src));
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_move(target_type& dst, void* src) {
        dst.template prepare_to_assign<T>();
        std::construct_at(dst.template data_as<T>(), std::move(*static_cast<T*>(src)));
    }

    template<size_t Size, size_t Align>
    template<typename T>
    void TAnyVtable<Size, Align>::impl_change_resource(target_type& obj, std::pmr::memory_resource* mr) {
        if (*mr == *obj.mr)
            return;

        // We should move heap's storage in case it is allocated.
        // But otherwise we can skip it entirely.

        if constexpr (!is_sbo_applicable_v<T, Size, Align>) {
            auto old_mr = obj.mr;

            void* old_heap = obj.storage.heap;
            void* new_heap = obj.mr->allocate(sizeof(T), alignof(T));

            if constexpr (std::is_trivially_copyable_v<T>)
                std::memcpy(new_heap, old_heap, sizeof(T));
            else
                std::construct_at(static_cast<T*>(new_heap), std::move(*static_cast<T*>(old_heap)));

            if constexpr (!std::is_trivially_destructible_v<T>)
                std::destroy_at(static_cast<T*>(old_heap));

            old_mr->deallocate(old_heap, sizeof(T), alignof(T));

            obj.storage.heap = new_heap;
        }

        obj.mr = mr;
    }
}
