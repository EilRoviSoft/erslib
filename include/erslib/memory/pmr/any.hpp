#pragma once

// std
#include <cstddef>
#include <memory_resource>

// erslib
#include <erslib/concept/sbo.hpp>
#include <erslib/type/general.hpp>


// Storage

namespace ers::internal {
    enum class TAnyState : u8 {
        Empty = 0,
        Dynamic,
        Embedded
    };

    template<size_t Size, size_t Align>
    struct TAnyStorage {
        static constexpr bool has_buffer = true;

        TAnyState state = TAnyState::Empty;
        union {
            alignas(Align) std::byte buffer[Size];
            const void* instance = nullptr;
        };
    };

    template<size_t Align>
    struct TAnyStorage<0, Align> {
        static constexpr bool has_buffer = false;

        TAnyState state = TAnyState::Empty;
        const void* instance = nullptr;
    };
}

// Vtable type-erased methods

namespace ers::internal {
    template<typename T>
    const type_info& info() noexcept {
        return typeid(T);
    }

    template<typename T, size_t Size, size_t Align>
    void impl_destroy(
        std::pmr::memory_resource* mr,
        TAnyStorage<Size, Align>& storage
    ) noexcept {
        if constexpr (is_sbo_applicable_v<T, Size, Align>) {
            std::destroy_at(reinterpret_cast<T*>(storage.buffer));
        } else {
            std::destroy_at(static_cast<T*>(storage.instance));
            mr->deallocate(storage.instance, sizeof(T), alignof(T));
        }
    }

    template<typename T, size_t Size, size_t Align>
        requires is_sbo_applicable_v<T, Size, Align>
    void impl_copy(
        std::pmr::memory_resource* mr,
        TAnyStorage<Size, Align>& lhs,
        const TAnyStorage<Size, Align>& rhs
    ) {
    }
}

// Vtable

namespace ers::internal {
    template<size_t Size, size_t Align>
    struct TAnyVtable {
        const type_info& (*info)() noexcept;
        void (*destroy)(std::pmr::memory_resource* mr, TAnyStorage<Size, Align>& storage) noexcept;
        void (*copy)(std::pmr::memory_resource* mr, TAnyStorage<Size, Align>& lhs, const TAnyStorage<Size, Align>& rhs);
        void (*move)(std::pmr::memory_resource* mr, TAnyStorage<Size, Align>& lhs, TAnyStorage<Size, Align>& rhs) noexcept;
    };

    template<typename T, size_t Size, size_t Align>
    auto make_vtable() noexcept {
        return TAnyVtable<Size, Align> {};
    }
}

namespace ers::pmr {
    template<size_t Size, size_t Align>
    class TAny {
    };
}
