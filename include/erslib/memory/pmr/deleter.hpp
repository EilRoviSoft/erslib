#pragma once

// std
#include <iostream>
#include <memory_resource>

namespace ers::pmr {
    template<class T>
    struct deleter {
        std::pmr::polymorphic_allocator<T> alloc;

        deleter() = default;
        explicit deleter(std::pmr::polymorphic_allocator<T> a) noexcept :
            alloc(a) {
        }

        void operator()(T* p) {
            if (!p)
                return;
            std::destroy_at(p);
            alloc.deallocate(p, 1);
        }
    };
}
