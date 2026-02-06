#pragma once

// std
#include <memory_resource>

namespace ers::pmr {
    template<class T>
    struct deleter {
        std::pmr::polymorphic_allocator<T> alloc;

        deleter() = default;
        explicit deleter(std::pmr::polymorphic_allocator<T> a) noexcept :
            alloc(a) {
        }

        void operator()(T* p) noexcept {
            if (!p)
                return;
            alloc.deallocate(p, 1);
        }
    };
}
