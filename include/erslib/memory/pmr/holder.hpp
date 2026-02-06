#pragma once

// std
#include <memory>

// erslib
#include <erslib/memory/pmr/deleter.hpp>

namespace ers::pmr {
    template<typename T>
    using Holder = std::unique_ptr<T, deleter<T>>;

    template<typename T, typename... Args>
    Holder<T> make_holder(std::pmr::memory_resource* resource, Args&&... args) {
        std::pmr::polymorphic_allocator<T> alloc(resource);

        T* p = alloc.allocate(1);
        try {
            alloc.construct(p, std::forward<Args>(args)...);
        } catch (...) {
            alloc.deallocate(p, 1);
            throw;
        }

        return Holder<T>(p, deleter<T>(alloc));
    }
}
