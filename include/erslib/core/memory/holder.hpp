#pragma once

// std
#include <memory>
#include <memory_resource>

// ers
#include <erslib/core/concept/util.hpp>
#include <erslib/core/memory/deleter.hpp>


namespace ers::impl {
    template<typename T>
    using holder_ptr = std::unique_ptr<T, deleter<T>>;


    template<typename T, typename... Args>
        requires (sizeof...(Args) == 0 || !ConvertibleToIn<std::pmr::memory_resource*, 0, Args...>)
    holder_ptr<T> make_holder(Args&&... args) {
        return holder_ptr<T>(new T(std::forward<Args>(args)...), deleter<T> {});
    }
    
    template<typename T, typename... Args>
    holder_ptr<T> make_holder(std::pmr::memory_resource* mr, Args&&... args) {
        std::pmr::polymorphic_allocator<T> alloc(mr);


        T* p = alloc.allocate(1);
        try {
            alloc.construct(p, std::forward<Args>(args)...);
        } catch (...) {
            alloc.deallocate(p, 1);
            throw;
        }


        return holder_ptr<T>(p, deleter<T>(mr));
    }


    template<typename T, typename Derived, typename... Args>
        requires std::derived_from<Derived, T>
    holder_ptr<T> make_polymorphic_holder(std::pmr::memory_resource* mr, Args&&... args) {
        std::pmr::polymorphic_allocator<Derived> alloc(mr);


        Derived* p = alloc.allocate(1);
        try {
            alloc.construct(p, std::forward<Args>(args)...);
        } catch (...) {
            alloc.deallocate(p, 1);
            throw;
        }


        return holder_ptr<T>(static_cast<T*>(p), deleter<T>(mr));
    }
}
