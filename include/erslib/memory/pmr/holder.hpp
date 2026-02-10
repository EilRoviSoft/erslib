#pragma once

// std
#include <memory>
#include <memory_resource>

// erslib
#include <erslib/memory/pmr/deleter.hpp>

namespace ers::pmr {
    template<typename T>
    using Holder = std::unique_ptr<T, deleter<T>>;

    template<typename T, typename... Args>
    Holder<T> make_holder(std::pmr::memory_resource* mr, Args&&... args) {
        std::pmr::polymorphic_allocator<T> alloc(mr);

        T* p = alloc.allocate(1);
        try {
            alloc.construct(p, std::forward<Args>(args)...);
        } catch (...) {
            alloc.deallocate(p, 1);
            throw;
        }

        return Holder<T>(p, deleter<T>(alloc));
    }

    template<typename T, typename Derived, typename... Args>
        requires std::derived_from<Derived, T>
    Holder<T> make_polymorphic_holder(std::pmr::memory_resource* mr, Args&&... args) {
        std::pmr::polymorphic_allocator<Derived> alloc(mr);

        Derived* p = alloc.allocate(1);
        try {
            alloc.construct(p, std::forward<Args>(args)...);
        } catch (...) {
            alloc.deallocate(p, 1);
            throw;
        }

        return Holder<T>(static_cast<T*>(p), deleter<T>(alloc));
    }
}
